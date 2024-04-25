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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "config.h"
#include "drv_cpu.h"
#include "drv_io.h"

// Flags for [u8i_interrupts] and [u8_buf_interrupts].
#define INTR_SYS_TICK		(1<<0)
#define INTR_READ_ADC		(1<<1)
#define INTR_TALLY			(1<<2)
#define INTR_SERIAL_DET		(1<<3)

// Flags for [u8_tasks].
#define	TASK_500HZ			(1<<0)	// 500 Hz event
#define	TASK_5HZ			(1<<1)	// 5 Hz event
#define	TASK_2HZ			(1<<2)	// 2 Hz event
#define	TASK_SLOW_BLINK		(1<<3)	// Indicator slow blink source
#define	TASK_FAST_BLINK		(1<<4)	// Indicator fast blink source
#define	TASK_CAMERA_OFF		(1<<5)	// Camera is not connected or is in power save mode
#define	TASK_LOW_BATT		(1<<6)	// Incoming power has too low voltage

// Voltage thresholds.
enum
{
	VIN_LOW_BATT	= 172,			// 11.0 V
};

// Flags for [u8_inputs].
#define	LINP_VTR_PB			(1<<0)	// VTR is in playback mode
#define	LINP_CAM_REC		(1<<1)	// Camera REC button was pressed
#define	LINP_CAM_RR			(1<<2)	// Camera RR button was pressed
#define	LINP_REC_LOCK		(1<<3)	// Record command lock (for impulse trigger)

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
	TIME_VTR_PB		= 200,			// 400 ms delay between playback/record switching
	TIME_CAM_REC	= 150,			// 300 ms delay for detecting triggered record signal
	TIME_SER_CLK	= 12,			// 96 us maximum time between falling edges in single byte transmittion
	TIME_SER_IB		= 64,			// 512 us maximum time between last bit of previous byte and first bit of current byte
	TIME_SER_MAX	= 250,			// marker for "timer has overflown and stopped"
};

// Serial commands.
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
	SCMD_NODEV		= 0b11111111,	// No device (camera) is present
};

#define	SER_PACK_LEN		7		// Number of bytes in single serial transmittion 
#define	SER_BYTE_LEN		8		// Number of bits in a byte
#define	SER_LAST_BIT		(SER_BYTE_LEN-1)

#endif /* SONY10P_H_ */
