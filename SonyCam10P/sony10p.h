/**************************************************************************************************************************************************************
sony10p.h

Copyright © 2024 Maksim Kryukov <fagear@mail.ru>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Created: 2024-04-21

**************************************************************************************************************************************************************/

#ifndef SONY10P_H_
#define SONY10P_H_

#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "config.h"
#include "drv_cpu.h"
#include "drv_io.h"

// Flags for [u8i_interrupts] and [u8_buf_interrupts].
#define INTR_SYS_TICK		(1<<0)	// System timing
#define INTR_READ_ADC		(1<<1)	// ADC result is ready
#define INTR_SERIAL			(1<<2)	// Detected serial link with NV-180
#define INTR_RX				(1<<3)	// Serial transmission finished
#define INTR_TALLY			(1<<4)	// Tally signal in Serial transmission 

// Flags for [u8_tasks].
#define	TASK_500HZ			(1<<0)	// 500 Hz event
#define	TASK_50HZ			(1<<1)	// 50 Hz event
#define	TASK_5HZ			(1<<2)	// 5 Hz event
#define	TASK_2HZ			(1<<3)	// 2 Hz event
#define	TASK_BATT_BLINK		(1<<4)	// Low battery blink
#define	TASK_FAST_BLINK		(1<<5)	// Indicator fast blink source
#define	TASK_SLOW_BLINK		(1<<6)	// Indicator slow blink source

// Flags for [u8_state].
#define	STATE_REC_LOCK		(1<<0)	// Record command lock (for impulse trigger)
#define	STATE_CAM_OFF		(1<<1)	// Camera is not connected or is in power save mode
#define	STATE_LOW_BATT		(1<<2)	// Incoming power has too low voltage
#define	STATE_SERIAL_DET	(1<<3)	// Serial link present

// Voltage thresholds.
enum
{
	VIN_LOW_BATT_UP	= 40,			// Input voltage 10.65 V ("battery OK")
	VIN_LOW_BATT_DN = 34,			// Input voltage 10.35 V ("battery low")
	VIN_OFF			= 13,			// Input voltage 9.5 V
	VD_CAM_ON_UP	= 65,			// Voltage difference for detected camera ON
	VD_CAM_ON_DN	= 55,			// Voltage difference for detected camera OFF
};
// Voltage difference when camera is operating: 34...41% duty of 8-bit PWM (87...105) of [u8_cam_pwr]
// Voltage difference when camera is operating w/o viewfinder: 31...35% (79...89) duty of 8-bit PWM
// Voltage difference when camera is connected, but in standby: 16...19% (41...48) duty of 8-bit PWM
// Voltage difference when camera is not connected: 9...15% duty of 8-bit PWM

// Length of the buffer for each ADC channel before filtering.
#define	ADC_HIST_LEN		49

// Flags for [u8_inputs].
#define	LINP_VTR_PB			(1<<0)	// VTR is in playback mode
#define	LINP_CAM_REC		(1<<1)	// Camera REC button was pressed
#define	LINP_CAM_RR			(1<<2)	// Camera RR button was pressed

// Flags for [u8_outputs].
#define	OUT_RLY_ON			(1<<0)	// Video selection relay is energized
#define	OUT_VTR_RUN			(1<<1)	// Make VTR run in record mode (not pause)
#define	OUT_VTR_STBY		(1<<2)	// Put NV-180 VTR in power save mode
#define	OUT_CAM_LED			(1<<3)	// Turn camera record LED on
#define	OUT_CAM_PB			(1<<4)	// Make camera display signal from video input

// Timeouts.
enum
{
	TIME_STARTUP	= 250,			// 500 ms startup delay
	TIME_VTR_PB		= 150,			// 300 ms delay between playback/record switching
	TIME_CAM_REC	= 125,			// 250 ms delay for detecting triggered record signal
	TIME_CAM_RR		= 250,			// 500 ms delay for transition between review and record pause
	TIME_REC_FADE	= 12,			// 6 s delay after record button state change before tally light blinking (~5 s fade in/out)
	TIME_CMD		= 100,			// 200 ms duration on the new serial command to the VTR
	TIME_SER_CLK	= 12,			// 96 us maximum time between falling edges in single byte transmission
	TIME_SER_IB		= 64,			// 512 us maximum time between last bit of previous byte and first bit of current byte
	TIME_SER_MAX	= 250,			// Marker for "timer has overflown and stopped"
	TIME_SER_C_INH	= 40,			// 20 s maximum time for VTR to go to recording mode from stop
	TIME_SER_C_RP	= 240,			// 120 s delay before going into standby from paused recording
	TIME_SER_C_ERR	= 2,			// 1 s delay before error mode within serial linked operation can be canceled
};

// Logic states for operating with serial link.
// TODO: Rec Review processing (unable to test due to faulty camera cable)
enum
{
	LST_STOP,						// Initial state before any record or if errored out of record mode
	LST_INH_CHECK,					// Record commanded from camera, checking if it is possible
	LST_REC_RDY,					// Recording granted, indicating, no actual recording
	LST_REC_PAUSE,					// Pause recording
	LST_RECORD,						// Recording
	LST_REC_PWRSV,					// Pause recording with powersave
	LST_ERROR,						// Errored out of normal operation (temporary mode)
};

// Error codes for operating with serial link.
enum
{
	ERR_ALL_OK,						// No error registered
	ERR_NO_TAPE,					// No tape in the VTR
	ERR_REC_INHIBIT,				// Inserted tape is protected from recording	
	ERR_REC_TIMEOUT,				// Unable to start recording in reasonable time
	ERR_CTRL_FAIL,					// Lost mode control of the VTR
};

#define	SER_BYTE_LEN		8		// Number of bits in a byte
#define	SER_LAST_BIT		(SER_BYTE_LEN-1)

// Byte offsets in one transmission consisting of [SER_PACK_LEN] bytes.
enum
{
	SER_CAM2VTR_OFS,				// Command from camera to VTR
	SER_TUN2VTR_OFS,				// Command from tuner/timer to VTR
	SER_REM2VTR_OFS,				// Command from wired remote to VTR
	SER_MN2UPD_OFS,					// Mode data from tape transport IC (MN1534) to display/logic IC (uPD7503)
	SER_VTR2CAM_DISP_OFS,			// VTR to camera display information (counter, battery, remote switch)
	SER_VTR2CAM_CNT1_OFS,			// VTR to camera tape counter or battery information, part 1
	SER_VTR2CAM_CNT2_OFS,			// VTR to camera tape counter or battery information, part 2
	SER_PACK_LEN					// Number of bytes in single serial transmission 
};

// Serial commands (from devices like camera to VTR).
enum
{
	SCMD_STOP		= 0b00000000,	// Stop transport
	SCMD_REW		= 0b00000010,	// Rewind tape
	SCMD_FF			= 0b00000011,	// Fast forward tape
	SCMD_REVIEW		= 0b00000100,	// Fast playback backwards
	SCMD_CUE		= 0b00000101,	// Fast playback forwards
	SCMD_STILL		= 0b00000110,	// Playback+pause
	SCMD_REC		= 0b00001000,	// Record
	SCMD_PLAY		= 0b00001010,	// Playback
	SCMD_STEP		= 0b00001100,	// Step one frame forward in pause
	SCMD_SLOW		= 0b00001111,	// Slow playback
	SCMD_SPD_UP		= 0b00101110,	// Step speed up for slow playback
	SCMD_SPD_DN		= 0b00101111,	// Step speed down for slow playback
	SCMD_STBY		= 0b10010110,	// Stand by
	SCMD_RCPB		= 0b10011010,	// Select record/playback
	SCMD_IDLE		= 0b11111111,	// No command (three in a row and VTR switches clock off)
};

// Values for [SER_VTR2CAM_DISP_OFS].
enum
{
	SDISP_CNTR_M	= 0b01010000,	// High nibble for tape counter with "M"(emory) mark ON transmission
	SDISP_CNTR		= 0b00000000,	// High nibble for tape counter no "M"(emory) mark transmission
	SDISP_BATT		= 0b11110000,	// High nibble for battery level transmission
	SDISP_REM_ON	= 0b00000000,	// Low nibble for camera remote switch ON
	SDISP_REM_OFF	= 0b00000101,	// Low nibble for camera remote switch OFF
};

// Bit flags for [SER_MN2UPD_OFS].
enum
{
	STTR_REC_INH	= (1<<7),		// Record inhibit switch is active
	STTR_LN_MASK	= 0x0F,			// Mask for the low nibble
	STTR_LN_STOP	= 0b00000000,	// Low nibble for STOP
	STTR_LN_EJECT	= 0b00000001,	// Low nibble for EJECT
	STTR_LN_REW		= 0b00000010,	// Low nibble for REWIND
	STTR_LN_FF		= 0b00000011,	// Low nibble for FAST FORWARD
	STTR_LN_REVIEW	= 0b00000100,	// Low nibble for SEARCH REVERSE
	STTR_LN_CUE		= 0b00000101,	// Low nibble for SEARCH FORWARD
	STTR_LN_STBY	= 0b00000111,	// Low nibble for STAND-BY
	STTR_LN_PLAY	= 0b00001000,	// Low nibble for PLAY
	STTR_LN_PLAY_P	= 0b00001001,	// Low nibble for PLAY+PAUSE
	STTR_LN_REC		= 0b00001010,	// Low nibble for RECORD
	STTR_LN_REC_P	= 0b00001011,	// Low nibble for RECORD+PAUSE
	STTR_LN_ADUB	= 0b00001100,	// Low nibble for AUDIO DUB
	STTR_LN_ADUB_P	= 0b00001101,	// Low nibble for AUDIO DUB+PAUSE
	STTR_LN_VADD	= 0b00001110,	// Low nibble for VIDEO ADD
	STTR_LN_VADD_P	= 0b00001111,	// Low nibble for VIDEO ADD+PAUSE
	STTR_HN_MASK	= 0xF0,			// Mask for the high nibble
	STTR_HN_S_STOP	= 0b00000000,	// High nibble for stable mechanical STOP (with a tape inside)
	STTR_HN_S_FAST	= 0b00010000,	// High nibble for stable mechanical EJECT/STOP (with no tape)/REWIND/FAST FORWARD
	STTR_HN_P_STOP	= 0b00100000,	// High nibble for moving to STOP
	STTR_HN_P_PLAY	= 0b00110000,	// High nibble for moving to PLAY/SLOW/SEARCH/STILL
	STTR_HN_M_RUN	= 0b01000000,	// High nibble for moving to PLAY (no video output yet)/REC (going to/from PAUSE)
	STTR_HN_S_RECP	= 0b01010000,	// High nibble for stable mechanical REC+PAUSE
	STTR_HN_S_REC	= 0b01100000,	// High nibble for stable mechanical RECORD
	STTR_HN_S_PLAY	= 0b01110000,	// High nibble for stable mechanical PLAY/SLOW/SEARCH/STILL
};

void sort_array(uint8_t *arr_ptr);
void load_serial_cmd(uint8_t new_cmd);
void go_to_rec_paused(void);
void go_to_powersave(void);
void go_to_error(uint8_t err_code);
int main(void);

#endif /* SONY10P_H_ */
