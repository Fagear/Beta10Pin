#include "sony10p.h"

volatile uint8_t u8i_interrupts = 0;		// Deferred interrupts call flags (non-buffered)
uint8_t u8_buf_interrupts = 0;				// Deferred interrupts call flags (buffered)
uint8_t u8_tasks=0;							// Deferred tasks call flags
uint8_t u8_state=0;							// Camera and VTR state
uint8_t u8_500hz_cnt=0;						// Divider for 500 Hz
uint8_t u8_50hz_cnt=0;						// Divider for 50 Hz
uint8_t u8_5hz_cnt=0;						// Divider for 5 Hz
uint8_t u8_2hz_cnt=0;						// Divider for 2 Hz
uint8_t u8_1hz_cnt=0;						// Divider for 1 Hz
uint8_t u8_blink_cnt=0;						// Blink counter (linked with [TASK_FAST_BLINK])
uint8_t u8_adc_12v=0;						// Voltage level at the power input
uint8_t u8_adc_cam=0;						// Voltage level at the camera output
uint8_t u8a_12v_hist[ADC_HIST_LEN];			// Last [ADC_HIST_LEN] values of [u8_adc_12v]
uint8_t u8a_camv_hist[ADC_HIST_LEN];		// Last [ADC_HIST_LEN] values of [u8_adc_cam]
uint8_t u8_adc_fill_ptr=0;					// Fill pointer for [u8a_12v_hist] and [u8a_camv_hist]
uint8_t u8_volt_12v=0;						// Filtered data from [u8_adc_12v]
uint8_t u8_volt_cam=0;						// Filtered data from [u8_adc_cam]
uint8_t u8_cam_pwr=0;						// Camera consumption (voltage difference)
uint8_t u8_inputs=0;						// Inputs state storage
uint8_t u8_outputs=0;						// Outputs state storage
uint8_t u8_start_dly=0;						// Start-up delay before processing anything
uint8_t u8_vid_dir_dly=0;					// Delay after detected video direction change
uint8_t u8_rec_trg_dly=0;					// Delay for detecting triggered record input
uint8_t u8_rr_dly=0;						// Delay between switching state between review/pause
uint8_t u8_rec_fade_dly=0;					// Delay after toggling record button state before tally light blinking allowed
#ifdef EN_SERIAL
uint8_t u8_ser_byte_cnt=0;					// Byte count in the serial packed transmission
uint8_t u8_ser_bit_cnt=0;					// Bit count in the serial packed transmission
volatile uint8_t u8a_ser_data[SER_PACK_LEN];// Data storage for serial transmission
volatile uint8_t u8_ser_cmd=SCMD_STOP;		// Command to be sent through serial link to NV-180 VTR
uint8_t u8_link_state=LST_STOP;				// State of operation through
#ifndef MCU_LOW_ROM
uint8_t u8_ser_error=ERR_ALL_OK;			// Last error during operating with serial link
#endif	/* MCU_LOW_ROM */
uint8_t u8_vtr_mode=0;						// Logic and mechanical mode of VTR, received through serial link
uint8_t u8_ser_cmd_dly=0;					// Duration for sending new command to the VTR
uint8_t u8_ser_mode_dly=0;					// Timer for serial link modes
uint8_t u8_ser_link_wd=0;					// Watchdog timer for serial link communication
#endif	/* EN_SERIAL */

// Using LUT for trading ROM space for computing time to speed up ADC read routine.
// LUT for 10-bit to 8-bit conversion to pick 9.0...15.0 V range.
const uint8_t ucaf_adc_to_byte[1024] PROGMEM =
{
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,
	1,	 1,	 2,	 2,	 2,	 3,	 3,	 4,
	4,	 4,	 5,	 5,	 6,	 6,	 7,	 7,
	7,	 8,	 8,	 9,	 9,	 9,	 10,	 10,
	11,	 11,	 11,	 12,	 12,	 13,	 13,	 13,
	14,	 14,	 15,	 15,	 15,	 16,	 16,	 17,
	17,	 17,	 18,	 18,	 19,	 19,	 20,	 20,
	20,	 21,	 21,	 22,	 22,	 22,	 23,	 23,
	24,	 24,	 24,	 25,	 25,	 26,	 26,	 26,
	27,	 27,	 28,	 28,	 28,	 29,	 29,	 30,
	30,	 30,	 31,	 31,	 32,	 32,	 33,	 33,
	33,	 34,	 34,	 35,	 35,	 35,	 36,	 36,
	37,	 37,	 37,	 38,	 38,	 39,	 39,	 39,
	40,	 40,	 41,	 41,	 41,	 42,	 42,	 43,
	43,	 43,	 44,	 44,	 45,	 45,	 46,	 46,
	46,	 47,	 47,	 48,	 48,	 48,	 49,	 49,
	50,	 50,	 50,	 51,	 51,	 52,	 52,	 52,
	53,	 53,	 54,	 54,	 54,	 55,	 55,	 56,
	56,	 57,	 57,	 57,	 58,	 58,	 59,	 59,
	59,	 60,	 60,	 61,	 61,	 61,	 62,	 62,
	63,	 63,	 63,	 64,	 64,	 65,	 65,	 65,
	66,	 66,	 67,	 67,	 67,	 68,	 68,	 69,
	69,	 70,	 70,	 70,	 71,	 71,	 72,	 72,
	72,	 73,	 73,	 74,	 74,	 74,	 75,	 75,
	76,	 76,	 76,	 77,	 77,	 78,	 78,	 78,
	79,	 79,	 80,	 80,	 80,	 81,	 81,	 82,
	82,	 83,	 83,	 83,	 84,	 84,	 85,	 85,
	85,	 86,	 86,	 87,	 87,	 87,	 88,	 88,
	89,	 89,	 89,	 90,	 90,	 91,	 91,	 91,
	92,	 92,	 93,	 93,	 93,	 94,	 94,	 95,
	95,	 96,	 96,	 96,	 97,	 97,	 98,	 98,
	98,	 99,	 99,	 100,	 100,	 100,	 101,	 101,
	102,	 102,	 102,	 103,	 103,	 104,	 104,	 104,
	105,	 105,	 106,	 106,	 107,	 107,	 107,	 108,
	108,	 109,	 109,	 109,	 110,	 110,	 111,	 111,
	111,	 112,	 112,	 113,	 113,	 113,	 114,	 114,
	115,	 115,	 115,	 116,	 116,	 117,	 117,	 117,
	118,	 118,	 119,	 119,	 120,	 120,	 120,	 121,
	121,	 122,	 122,	 122,	 123,	 123,	 124,	 124,
	124,	 125,	 125,	 126,	 126,	 126,	 127,	 127,
	128,	 128,	 128,	 129,	 129,	 130,	 130,	 130,
	131,	 131,	 132,	 132,	 133,	 133,	 133,	 134,
	134,	 135,	 135,	 135,	 136,	 136,	 137,	 137,
	137,	 138,	 138,	 139,	 139,	 139,	 140,	 140,
	141,	 141,	 141,	 142,	 142,	 143,	 143,	 143,
	144,	 144,	 145,	 145,	 146,	 146,	 146,	 147,
	147,	 148,	 148,	 148,	 149,	 149,	 150,	 150,
	150,	 151,	 151,	 152,	 152,	 152,	 153,	 153,
	154,	 154,	 154,	 155,	 155,	 156,	 156,	 157,
	157,	 157,	 158,	 158,	 159,	 159,	 159,	 160,
	160,	 161,	 161,	 161,	 162,	 162,	 163,	 163,
	163,	 164,	 164,	 165,	 165,	 165,	 166,	 166,
	167,	 167,	 167,	 168,	 168,	 169,	 169,	 170,
	170,	 170,	 171,	 171,	 172,	 172,	 172,	 173,
	173,	 174,	 174,	 174,	 175,	 175,	 176,	 176,
	176,	 177,	 177,	 178,	 178,	 178,	 179,	 179,
	180,	 180,	 180,	 181,	 181,	 182,	 182,	 183,
	183,	 183,	 184,	 184,	 185,	 185,	 185,	 186,
	186,	 187,	 187,	 187,	 188,	 188,	 189,	 189,
	189,	 190,	 190,	 191,	 191,	 191,	 192,	 192,
	193,	 193,	 193,	 194,	 194,	 195,	 195,	 196,
	196,	 196,	 197,	 197,	 198,	 198,	 198,	 199,
	199,	 200,	 200,	 200,	 201,	 201,	 202,	 202,
	202,	 203,	 203,	 204,	 204,	 204,	 205,	 205,
	206,	 206,	 207,	 207,	 207,	 208,	 208,	 209,
	209,	 209,	 210,	 210,	 211,	 211,	 211,	 212,
	212,	 213,	 213,	 213,	 214,	 214,	 215,	 215,
	215,	 216,	 216,	 217,	 217,	 217,	 218,	 218,
	219,	 219,	 220,	 220,	 220,	 221,	 221,	 222,
	222,	 222,	 223,	 223,	 224,	 224,	 224,	 225,
	225,	 226,	 226,	 226,	 227,	 227,	 228,	 228,
	228,	 229,	 229,	 230,	 230,	 230,	 231,	 231,
	232,	 232,	 233,	 233,	 233,	 234,	 234,	 235,
	235,	 235,	 236,	 236,	 237,	 237,	 237,	 238,
	238,	 239,	 239,	 239,	 240,	 240,	 241,	 241,
	241,	 242,	 242,	 243,	 243,	 243,	 244,	 244,
	245,	 245,	 246,	 246,	 246,	 247,	 247,	 248,
	248,	 248,	 249,	 249,	 250,	 250,	 250,	 251,
	251,	 252,	 252,	 252,	 253,	 253,	 254,	 254
};


// Firmware description strings.
volatile const uint8_t ucaf_version[] PROGMEM = "v0.05";			// Firmware version
volatile const uint8_t ucaf_compile_time[] PROGMEM = __TIME__;		// Time of compilation
volatile const uint8_t ucaf_compile_date[] PROGMEM = __DATE__;		// Date of compilation
volatile const uint8_t ucaf_info[] PROGMEM = "Sony Beta camera 14-pin to 10-pin EIAJ adapter";	// Firmware description
volatile const uint8_t ucaf_author[] PROGMEM = "Maksim Kryukov aka Fagear (fagear@mail.ru)";	// Author

//-------------------------------------- System timer interrupt handler.
ISR(SYST_INT, ISR_NAKED)
{
	INTR_IN;
	// 1000 Hz event.
	u8i_interrupts |= INTR_SYS_TICK;
	INTR_OUT;
}

//-------------------------------------- ADC interrupt handler.
ISR(ADC_INT)
{
	//u8i_interrupts |= INTR_READ_ADC;
	uint8_t mux, data;
	// Read last mux state.
	mux = ADC_MUX_RD;
	// Clear selected channel.
	ADC_MUX_CLR;
	// Read data (converting from 10-bit to 8-bit).
	data = pgm_read_byte_near(ucaf_adc_to_byte+ADC_DATA);
	if(mux==ADC_CH_12V)
	{
		// Save input 12V voltage level.
		u8_adc_12v = data;
		// Switch to next ADC channel.
		ADC_MUX_WR |= ADC_CH_CAM;
	}
	else
	{
		// Save output 12V to camera voltage level.
		u8_adc_cam = data;
		// Switch to next ADC channel.
		ADC_MUX_WR |= ADC_CH_12V;
	}
}

#ifdef EN_SERIAL
//-------------------------------------- Serial link clock pin change interrupt handler.
ISR(VTR_SER_INT)
{
	uint8_t timer_data;
	// Save timer count.
	timer_data = SERT_DATA_8;
	// Clear and restart timer.
	SERT_RESET; SERT_START;
	// Reset serial link watchdog.
	u8_ser_link_wd = TIME_SER_TO;
	// Check for rising or falling edge.
	if(VTR_SCLK_STATE==0)
	{
		// Falling edge.
		// Start of a new pulse.
		if(timer_data<TIME_SER_CLK)
		{
			// Pulse within single-byte transmission.
			// Switch to the next bit.
			u8_ser_bit_cnt = (u8_ser_bit_cnt>>1);
			// By default, byte offset [SER_CAM2VTR_OFS] is used for TX from camera, all other byte for RX.
			if(u8_ser_byte_cnt==SER_CAM2VTR_OFS)
			{
				// Preset next data bit for output to be read at rising edge by VTR.
				if((u8a_ser_data[SER_CAM2VTR_OFS]&u8_ser_bit_cnt)==0)
				{
					VTR_SDAT_SET0;
				}
				else
				{
					VTR_SDAT_SET1;
				}
			}
		}
		else if(timer_data<TIME_SER_IB)
		{
			// Start of the new byte (2nd+) in packed transmission.
			// Switch data pin to input.
			VTR_SDAT_IN; VTR_SDAT_SET1;
			// Check transmission array boundaries.
			if(u8_ser_byte_cnt<SER_PACK_LEN)
			{
				// Switch to the next byte.
				u8_ser_byte_cnt++;
				// Clear next byte in the buffer for TX.
				u8a_ser_data[u8_ser_byte_cnt] = 0;
				// Start from the bit 7 (MSB first).
				u8_ser_bit_cnt = (1<<SER_LAST_BIT);
			}
		}
		else
		{
			// Start of the new packed transmission after a pause.
			// Indicate start of the transmission.
			DBG_3_ON;
			// Reset bit (MSB first) and byte counters.
			u8_ser_byte_cnt = 0;
			u8_ser_bit_cnt = (1<<SER_LAST_BIT);
			// Check if serial link was detected on the first transmission from VTR.
			if((u8i_interrupts&INTR_SERIAL)!=0)
			{
				// Preset data bit 7 for output to be read at rising edge by VTR.
				if((u8a_ser_data[SER_CAM2VTR_OFS]&u8_ser_bit_cnt)==0)
				{
					VTR_SDAT_SET0;
				}
				else
				{
					VTR_SDAT_SET1;
				}
				// Enable output from data pin.
				VTR_SDAT_OUT;
			}
		}
	}
	else
	{
		// Rising edge.
		// Pulse half-way, data sampling point.
		if(timer_data<TIME_SER_CLK)
		{
			// Pulse within single-byte transmission.
			if((u8_ser_byte_cnt>SER_CAM2VTR_OFS)&&(u8_ser_byte_cnt<SER_PACK_LEN))
			{
				// Fill array only on byte number 2...7 (offset 1...6).
				if(VTR_SDAT_STATE!=0)
				{
					// Set current data bit.
					u8a_ser_data[u8_ser_byte_cnt] |= u8_ser_bit_cnt;
				}
			}
		}
	}
}

//-------------------------------------- Serial link timing interrupt handler.
ISR(SERT_INT)
{
	// Stop count on overflow.
	SERT_STOP;
	// Preset "overflow and stopped" value.
	SERT_DATA_8 = TIME_SER_MAX;
	// Check clock line level on timeout.
	if(VTR_SCLK_STATE==0)
	{
		// Tally signal detected.
		u8i_interrupts |= INTR_TALLY;
	}
	else
	{
		// No tally signal.
		u8i_interrupts &= ~INTR_TALLY;
	}
	// Load a command into the buffer to be sent in next transmission.
	u8a_ser_data[SER_CAM2VTR_OFS] = u8_ser_cmd;
	// Check if transmission took place and finished ok.
	if(u8_ser_byte_cnt>=(SER_PACK_LEN-1))
	{
		// Lock in presence of serial link and set a flag for finished transmission.
		u8i_interrupts |= (INTR_SERIAL|INTR_RX);
	}
	else if(VTR_SCLK_STATE!=0)
	{
		// Timed out with clock signal pulled high.
		// Serial link in not established.
		u8i_interrupts &= ~(INTR_SERIAL|INTR_TALLY);
	}
	// Indicate end of the transmission.
	DBG_3_OFF;
}
#endif	/* EN_SERIAL */

//-------------------------------------- Startup init.
static inline void system_startup(void)
{
	// Shut down watchdog.
	// Prevent mis-configured watchdog loop reset.
	wdt_reset();
	cli();
	WDT_RESET_DIS;
	WDT_PREP_OFF;
	WDT_SW_OFF;
	WDT_FLUSH_REASON;

	// Init hardware resources.
	HW_init();

	// Start system timer.
	SYST_START;

	// Enable watchdog (reset in ~1 s).
	WDT_PREP_ON;
	WDT_SW_ON;
	wdt_reset();
}

//-------------------------------------- Slow events dividers.
static inline void soft_timer_management(void)
{
	u8_500hz_cnt++;
	if(u8_500hz_cnt>=2)			// 1000/2 = 500.
	{
		u8_500hz_cnt = 0;
		// 500 Hz event.
		u8_tasks |= TASK_500HZ;

		u8_50hz_cnt++;
		if(u8_50hz_cnt>=10)		// 500/10 = 50.
		{
			u8_50hz_cnt = 0;
			// 50 Hz event.
			u8_tasks |= TASK_50HZ;

			u8_5hz_cnt++;
			if(u8_5hz_cnt==5)
			{
				// 5 Hz event, phase 2.
				u8_tasks |= TASK_5HZ_PH2;
				// Turn off fast blink.
				u8_tasks &= ~TASK_FAST_BLINK;
				// Increase blink counter.
				u8_blink_cnt++;
				if(u8_blink_cnt>0)
				{
					// Turn off low batt blink 10% duty cycle.
					u8_tasks &= ~TASK_BATT_BLINK;
				}
			}
			if(u8_5hz_cnt>=10)	// 50/10 = 5.
			{
				u8_5hz_cnt = 0;
				// 5 Hz event, phase 1.
				// Turn on fast blink.
				u8_tasks |= TASK_5HZ_PH1|TASK_FAST_BLINK;
				// Toggle slow blink.
				u8_tasks ^= TASK_SLOW_BLINK;
			}
			u8_2hz_cnt++;
			if(u8_2hz_cnt>=25)	// 50/25 = 2.
			{
				u8_2hz_cnt = 0;
				// 2 Hz event.
				u8_tasks |= TASK_2HZ;
			}
			u8_1hz_cnt++;
			if(u8_1hz_cnt>=50)	// 50/50 = 1.
			{
				u8_1hz_cnt = 0;
				// 1 Hz event.
				// Turn on low batt blink.
				u8_tasks |= TASK_BATT_BLINK;
				// Reset blink count
				u8_blink_cnt = 0;
			}
		}
	}
}

//-------------------------------------- Check state of input power supply.
static inline void input_power_check(void)
{
	// Don't check if input voltage channel is not read yet.
	if(u8_volt_12v==0)
	{
		return;
	}
	// Check if "low battery" indication should be lit.
	if((u8_state&STATE_LOW_BATT)==0)
	{
		// "Low battery" flag was NOT active before.
		// Check for lower threshold (for hysteresis).
		if(u8_volt_12v<=VIN_LOW_BATT_DN)
		{
			u8_state |= STATE_LOW_BATT;
		}
	}
	else
	{
		// "Low battery" flag WAS active before.
		// Check for higher threshold (for hysteresis).
		if(u8_volt_12v>=VIN_LOW_BATT_UP)
		{
			u8_state &= ~STATE_LOW_BATT;
		}
	}
}

//-------------------------------------- Determine camera power consumption.
static inline void camera_power_check(void)
{
#ifdef EN_CAM_PWR_DEF	
	u8_cam_pwr = 0;
	// Don't calculate consumption if one of the ADC channels is not read yet.
	if((u8_volt_12v==0)||(u8_volt_cam==0))
	{
		return;
	}
	// Prevent overflow if voltage dividers/ADC tolerances lead to impossible.
	if(u8_volt_12v>=u8_volt_cam)
	{
		// Calculate camera consumption.
		u8_cam_pwr = u8_volt_12v - u8_volt_cam;
	}
	if(u8_cam_pwr<32)
	{
		u8_cam_pwr = (u8_cam_pwr*8);
	}
	else
	{
		u8_cam_pwr = 254;
	}
	// Check if camera is powered on.
	if((u8_state&STATE_CAM_OFF)==0)
	{
		if(u8_cam_pwr<=VD_CAM_ON_DN)
		{
			u8_state |= STATE_CAM_OFF;
		}
	}
	else
	{
		if(u8_cam_pwr>=VD_CAM_ON_UP)
		{
			u8_state &= ~STATE_CAM_OFF;
		}
	}
#endif /* EN_CAM_PWR_DEF */
}

//-------------------------------------- Process timeouts.
static inline void delay_management(void)
{
	// Count down every 2 ms.
	if(u8_start_dly!=0) u8_start_dly--;
	if(u8_vid_dir_dly!=0) u8_vid_dir_dly--;
	if(u8_rec_trg_dly!=0) u8_rec_trg_dly--;
	if(u8_rr_dly!=0) u8_rr_dly--;
#ifdef EN_SERIAL
	// Serial command timeout.
	if(u8_ser_cmd_dly!=0)
	{
		u8_ser_cmd_dly--;
	}
	else
	{
		// Expire command, go idle.
		u8_ser_cmd = SCMD_STOP;
	}
	// Serial link watchdog.
	if(u8_ser_link_wd!=0)
	{
		u8_ser_link_wd--;
	}
	else
	{
		// No serial activity.
		u8i_interrupts &= ~INTR_SERIAL;
	}

#endif	/* EN_SERIAL */
}

//-------------------------------------- Process standby timers.
static inline void slow_state_timing(void)
{
	// Count down every 500 ms.
	if(u8_rec_fade_dly!=0) u8_rec_fade_dly--;
#ifdef EN_SERIAL
	if(u8_ser_mode_dly!=0) u8_ser_mode_dly--;
#endif	/* EN_SERIAL */
}

//-------------------------------------- Read all logic inputs.
static inline void read_inputs(void)
{
	if(u8_start_dly!=0)
	{
		// Don't process inputs on startup.
		return;
	}
	if(VTR_VID_PB)
	{
		// Check if previous state was different.
		if((u8_inputs&LINP_VTR_PB)==0)
		{
			// Check if timeout is done.
			if(u8_vid_dir_dly==0)
			{
				// Update to a new state.
				u8_inputs |= LINP_VTR_PB;
				// Reset timer to block updates.
				u8_vid_dir_dly = TIME_VTR_PB;
			}
		}
	}
	else
	{
		// Check if previous state was different.
		if((u8_inputs&LINP_VTR_PB)!=0)
		{
			// Check if timeout is done.
			if(u8_vid_dir_dly==0)
			{
				// Update to a new state.
				u8_inputs &= ~LINP_VTR_PB;
				// Reset timer to block updates.
				u8_vid_dir_dly = TIME_VTR_PB;
			}
		}
	}
	if(CAM_REC_LOW)
	{
		// Camera record button signal is in low state.
		// Check if previous state was different.
		if((u8_inputs&LINP_CAM_REC)==0)
		{
			// Previous signal state was low (inverted to actual signal from camera).
			// Falling edge.
			// Update to a new state.
			u8_inputs |= LINP_CAM_REC;
			// Reset blinking delay.
			u8_rec_fade_dly = TIME_REC_FADE;
			// Reset timer to catch triggered input.
			u8_rec_trg_dly = TIME_CAM_REC;
			// Toggle pause flag/record lock.
			if((u8_state&STATE_REC_LOCK)==0)
			{
				// Start "record" period.
				u8_state |= STATE_REC_LOCK;
			}
			else
			{
				// Stop "record" period.
				u8_state &= ~STATE_REC_LOCK;
			}
		}
	}
	else
	{
		// Camera record button signal is in high state.
		if((u8_inputs&LINP_CAM_REC)!=0)
		{
			// Previous signal state was high (inverted to actual signal from camera).
			// Rising edge.
			// Update to a new state.
			u8_inputs &= ~LINP_CAM_REC;
			// Check if trigger lock run out.
			if(u8_rec_trg_dly==0)
			{
				// Triggered record signal is not detected.
				// (trigger is 200 ms negative pulse,
				// normal human can not press record button twice that fast)
				// Treat input signal as direct level control.
				// Reset blinking delay.
				u8_rec_fade_dly = TIME_REC_FADE;
				// Toggle pause flag/record lock.
				if((u8_state&STATE_REC_LOCK)==0)
				{
					// Start "record" period.
					u8_state |= STATE_REC_LOCK;
				}
				else
				{
					// Stop "record" period.
					u8_state &= ~STATE_REC_LOCK;
				}
			}
		}
	}
	if(CAM_RR_DOWN)
	{
		// Camera record/review button signal is in low state.
		// Check if previous state was different.
		if((u8_inputs&LINP_CAM_RR)==0)
		{
			// Check if timeout is done.
			if(u8_rr_dly==0)
			{
				// Update to a new state.
				u8_inputs |= LINP_CAM_RR;
				// Reset timer to block updates.
				u8_rr_dly = TIME_CAM_RR;
			}
		}
	}
	else
	{
		// Camera record/review button signal is in high state.
		// Check if previous state was different.
		if((u8_inputs&LINP_CAM_RR)!=0)
		{
			// Check if timeout is done.
			if(u8_rr_dly==0)
			{
				// Update to a new state.
				u8_inputs &= ~LINP_CAM_RR;
				// Reset timer to block updates.
				u8_rr_dly = TIME_CAM_RR;
			}
		}
	}
}

//-------------------------------------- Sort. Array. What else?
void sort_array(uint8_t *arr_ptr)
{
	uint8_t tmp_val;
	// Perform bubble sort. :D
	for(uint8_t idx=0;idx<=((ADC_HIST_LEN/2)+1);idx++)
	{
		for(uint8_t cyc=0;cyc<(ADC_HIST_LEN-idx-1);cyc++)
		{
			if(arr_ptr[cyc]>arr_ptr[cyc+1])
			{
				tmp_val = arr_ptr[cyc];
				arr_ptr[cyc] = arr_ptr[cyc+1];
				arr_ptr[cyc+1] = tmp_val;
			}
		}
	}
}

//-------------------------------------- Buffer ADC values.
static inline void buffer_adc(void)
{
	// Update next values in history.
	u8a_12v_hist[u8_adc_fill_ptr] = u8_adc_12v;
	u8a_camv_hist[u8_adc_fill_ptr] = u8_adc_cam;
	// Loop fill pointer.
	u8_adc_fill_ptr++;
	if(u8_adc_fill_ptr>=ADC_HIST_LEN)
	{
		u8_adc_fill_ptr = 0;
	}
}

//-------------------------------------- Filter ADC values (phase 1).
//-------------------------------------- Run time ~800 us @ 8 MHz.
static inline void filter_adc_ph1(void)
{
	uint8_t sort_vals[ADC_HIST_LEN];
	// Copy first array.
	memcpy(sort_vals, u8a_12v_hist, ADC_HIST_LEN);
	// Sort the data.
	sort_array(sort_vals);
	// Pick center one (median filter).
	u8_volt_12v = sort_vals[(ADC_HIST_LEN/2)];
}

//-------------------------------------- Filter ADC values (phase 2).
//-------------------------------------- Run time ~800 us @ 8 MHz.
static inline void filter_adc_ph2(void)
{
	uint8_t sort_vals[ADC_HIST_LEN];
	// Copy second array.
	memcpy(sort_vals, u8a_camv_hist, ADC_HIST_LEN);
	// Sort the data.
	sort_array(sort_vals);
	// Pick center one (median filter).
	u8_volt_cam = sort_vals[(ADC_HIST_LEN/2)];
}

#ifdef EN_SERIAL
//-------------------------------------- Upload new command to the VTR.
void load_serial_cmd(uint8_t new_cmd)
{
	// Buffer new command.
	u8_ser_cmd = new_cmd;
	// Reset timer for duration.
	u8_ser_cmd_dly = TIME_CMD;
}

//-------------------------------------- Set serial link state to trying to start recording.
static inline void go_to_prep_rec(void)
{
	// Load maximum duration for [LST_INH_CHECK] mode.
	u8_ser_mode_dly = TIME_SCMD_INH;
	// Go to "check record inhibit mode".
	u8_link_state = LST_INH_CHECK;
}

//-------------------------------------- Set serial link state to paused recording.
static inline void go_to_rec_paused(void)
{
	// Set maximum allowed time in pause.
	u8_ser_mode_dly = TIME_REC_P_MAX;
	// Move to paused recording mode.
	u8_link_state = LST_REC_PAUSE;
}

//-------------------------------------- Set serial link state to powersaved paused recording.
void go_to_powersave(void)
{
	// Clear record lock to stay in powersave.
	u8_state &= ~STATE_REC_LOCK;
	// To go powersave mode.
	u8_link_state = LST_REC_PWRSV;
}

//-------------------------------------- Set serial link state to switching to playback.
static inline void go_to_switch_to_play(void)
{
	// Set maximum allowed time for mode transition.
	u8_ser_mode_dly = TIME_SCMD_2REC;
	// Switching from record to playback.
	u8_link_state = LST_SW_PB;
}

//-------------------------------------- Set serial link state to paused playback.
static inline void go_to_play_pause(void)
{
	// Set delay for holding in this mode.
	u8_ser_mode_dly = TIME_SCMD_REV_I;
	// Move to paused playback mode.
	u8_link_state = LST_PB_PAUSE;
}

//-------------------------------------- Set serial link state to paused playback.
static inline void go_to_play_pause_hold(void)
{
	// Set delay for holding in this mode.
	u8_ser_mode_dly = TIME_SCMD_REV_O;
	// Move to paused playback mode.
	u8_link_state = LST_PB_HOLD;
}

//-------------------------------------- Set serial link state to switching to record.
static inline void go_to_switch_to_record(void)
{
	// Set maximum allowed time for mode transition.
	u8_ser_mode_dly = TIME_SCMD_2REC;
	// Switching from playback to record.
	u8_link_state = LST_SW_REC;
}

//-------------------------------------- Set serial link state to error.
void go_to_error(uint8_t err_code)
{
	// Set record lock as a flag for holding in error mode.
	u8_state |= STATE_REC_LOCK;
	// Load error-exit delay.
	u8_ser_mode_dly = TIME_SCMD_ERR;
	// Go to error mode.
	u8_link_state = LST_ERROR;
#ifndef MCU_LOW_ROM
	// Save last error code.
	u8_ser_error = err_code;
#endif	/* MCU_LOW_ROM */
}
#endif	/* EN_SERIAL */

//-------------------------------------- Process inputs and generate outputs.
static inline void state_machine(void)
{
#ifdef EN_SERIAL
	// Check if serial link is present.
	if((u8_state&STATE_SERIAL_DET)!=0)
	{
		// Serial link is present, operating in serial linked mode.
		// Check video direction managed by VTR.
		if((u8_inputs&LINP_VTR_PB)!=0)
		{
			// VTR switched into playback mode.
			// Switch video path from VTR to camera.
			u8_outputs |= (OUT_RLY_ON|OUT_CAM_PB);
		}
		else
		{
			// VTR exited playback mode.
			// Switch video path from camera to VTR.
			u8_outputs &= ~(OUT_RLY_ON|OUT_CAM_PB);
		}
		// Don't use wired standby control method.
		// (it will work but it's not needed)
		u8_outputs &= ~OUT_VTR_STBY;
		// Perform state machine stuff.
		if(u8_link_state==LST_STOP)
		{
			// Should be in STOP now.
			if((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_STOP)
			{
				// Put VTR in standby to preserve energy.
				load_serial_cmd(SCMD_STBY);
			}
			else
			{
				// Force VTR to STOP.
				load_serial_cmd(SCMD_STOP);
			}
			// Check if state needs to be changed.
			// Check user input.
			if((u8_state&STATE_REC_LOCK)!=0)
			{
				// Record commanded from camera, try to initiate it.
				go_to_prep_rec();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Check battery level.
				if((u8_state&STATE_LOW_BATT)!=0)
				{
					// Blink LED on the camera (mostly off, since recording is off).
					if((u8_tasks&TASK_BATT_BLINK)!=0)
					{
						u8_outputs |= OUT_CAM_LED;
					}
				}
			}
		}
		else if(u8_link_state==LST_INH_CHECK)
		{
			// Trying to start recording if it is possible.
			load_serial_cmd(SCMD_REC);
			// Check if state needs to be changed.
			// Check if camera is present.
			if((u8_state&STATE_CAM_OFF)!=0)
			{
				// No camera, cancel recording.
				u8_link_state = LST_STOP;
			}
			// Check if VTR can record.
			else if(((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_FAST)&&
					((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_STOP))
			{
				// No tape in VTR.
				go_to_error(ERR_NO_TAPE);
			}
			else if((u8_vtr_mode&STTR_REC_INH)!=0)
			{
				// Recording inhibited by the safety switch.
				// (tab is broken off the cassette)
				go_to_error(ERR_REC_INHIBIT);
			}
			// Check how much time is already spent trying to initiate recording.
			else if(u8_ser_mode_dly==0)
			{
				// Timer run out.
				// VTR took too long to switch to record mode, something is up.
				go_to_error(ERR_MODE_TIMEOUT);
			}
			// Check user input.
			else if((u8_state&STATE_REC_LOCK)==0)
			{
				// Record command got deasserted during wait.
				// Return to STOP.
				u8_link_state = LST_STOP;
			}
			// Check if VTR settled in record+pause mode.
			else if(((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_RECP)||
					((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_REC))
			{
				// Progress to the "recording ready" state.
				u8_link_state = LST_REC_RDY;
			} 
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Blink tally light fast in preparation.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					u8_outputs |= OUT_CAM_LED;
				}
			}
		}
		else if(u8_link_state==LST_REC_RDY)
		{
			// Recording ready, all requirements are met.
			// Keep recording mode.
			load_serial_cmd(SCMD_REC);
			// Check if state needs to be changed.
			// Check if recording lock was cleared.
			if((u8_state&STATE_REC_LOCK)==0)
			{
				// Move to paused recording mode.
				go_to_rec_paused();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Try to clear recording lock to move to paused recording automatically.
				u8_state &= ~STATE_REC_LOCK;
			}
		}
		else if(u8_link_state==LST_REC_PAUSE)
		{
			// Paused recording.
			// Keep recording mode.
			load_serial_cmd(SCMD_REC);
			// Check if state needs to be changed.
			// Check if camera is present.
			if((u8_state&STATE_CAM_OFF)!=0)
			{
				// No camera.
				go_to_powersave();
			}
			// Check mechanical mode.
			else if(((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_M_RUN)&&
					((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_RECP)&&
					((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_REC))
			{
				// VTR dropped out from recording mode for some reason.
				go_to_error(ERR_CTRL_FAIL);
			}
			// Check how much time spent in paused state.
			else if(u8_ser_mode_dly==0)
			{
				// Pause is too long, tape is wearing out, battery is draining.
				go_to_powersave();
			}
			// Check user input.
			else if((u8_state&STATE_REC_LOCK)!=0)
			{
				// Proceed to normal recording.
				u8_link_state = LST_RECORD;
			}
			else if((u8_inputs&LINP_CAM_RR)!=0)
			{
				// RR button pressed.
				// Switch from record to playback.
				go_to_switch_to_play();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Check mechanical mode again.
				if((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_REC_P)
				{
					// Record+pause mode reached.
					// Check battery level.
					if((u8_state&STATE_LOW_BATT)!=0)
					{
						// Low battery condition detected.
						if(u8_rec_fade_dly==0)
						{
							// Possible fader must be done now.
							// Blink LED on the camera (mostly off, since recording is off).
							if((u8_tasks&TASK_BATT_BLINK)!=0)
							{
								u8_outputs |= OUT_CAM_LED;
							}
						}
					}
				}
			}
		}
		else if(u8_link_state==LST_RECORD)
		{
			// Normal recording.
			// Keep recording mode.
			load_serial_cmd(SCMD_REC);
			// Check if state needs to be changed.
			// Check if camera is present.
			if((u8_state&STATE_CAM_OFF)!=0)
			{
				// No camera.
				go_to_powersave();
			}
			// Check mechanical mode.
			else if(((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_M_RUN)&&
					((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_RECP)&&
					((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_REC))
			{
				// VTR dropped out from recording mode for some reason.
				go_to_error(ERR_CTRL_FAIL);
			}
			// Check user input.
			else if((u8_state&STATE_REC_LOCK)==0)
			{
				// Move to paused recording mode.
				go_to_rec_paused();
			}
			else
			{
				// Make sure that VTR got that memo about unpausing record mode.
				// Wait for VTR to get stable "rec+pause" mode set before removing "pause" signal.
				// (when VTR returns from standby it ignores pause pin in camera connector
				// until VTR re-initializes record mode
				// and then it gets stuck in record+pause with pin telling to "run")
				if((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_REC_P)
				{
					// Disable pause.
					u8_outputs |= OUT_VTR_RUN;
				}
				else if((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_REC)
				{
					// // Disable pause and enable tally light.
					u8_outputs |= (OUT_VTR_RUN|OUT_CAM_LED);
					// Check battery level.
					if((u8_state&STATE_LOW_BATT)!=0)
					{
						// Low battery condition detected.
						if(u8_rec_fade_dly==0)
						{
							// Possible fader must be done now.
							// Blink LED on the camera (mostly on, since recording is on).
							if((u8_tasks&TASK_BATT_BLINK)!=0)
							{
								u8_outputs &= ~OUT_CAM_LED;
							}
						}
					}
				}
			}
		}
		else if(u8_link_state==LST_REC_PWRSV)
		{
			// Paused recording with powersave.
			// Put VTR in standby.
			load_serial_cmd(SCMD_STBY);
			// Check if state needs to be changed.
			// Check mechanical mode.
			if(((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_M_RUN)&&
				((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_RECP)&&
				((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_REC))
			{
				// VTR dropped out from recording mode for some reason.
				go_to_error(ERR_CTRL_FAIL);
			}
			// Check user input.
			else if((u8_state&STATE_REC_LOCK)!=0)
			{
				// Proceed to normal recording.
				u8_link_state = LST_RECORD;
			}
			else if((u8_inputs&LINP_CAM_RR)!=0)
			{
				// RR button pressed.
				// Switch from record to playback.
				go_to_switch_to_play();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
			}
		}
		else if(u8_link_state==LST_SW_PB)
		{
			// Start transition from recording+pause to playback+pause.
			// Switch VTR to playback.
			load_serial_cmd(SCMD_REC2PB);
			// Check if state needs to be changed.
			// Check how much time is already spent trying to switch mode.
			if(u8_ser_mode_dly==0)
			{
				// Timer run out.
				// VTR took too long to switch to playback mode, something is up.
				go_to_error(ERR_MODE_TIMEOUT);
			}
			// Check if VTR settled in playback+pause.
			else if(((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_PLAY_P)&&
					((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_PLAY))
			{
				// Progress to the "pause preview" state.
				go_to_play_pause();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Blink tally light fast in preparation.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					u8_outputs |= OUT_CAM_LED;
				}
			}
		}
		else if(u8_link_state==LST_PB_PAUSE)
		{
			// Paused playback before backwards search.
			load_serial_cmd(SCMD_STILL);
			// Check if state needs to be changed.
			// Check how much time is already spent in this holding mode.
			if(u8_ser_mode_dly==0)
			{
				// Timer run out.
				// Finally, switch to reverse playback.
				u8_link_state = LST_PB_REW;
			}
			// Check mechanical mode.
			else if(((u8_vtr_mode&STTR_LN_MASK)!=STTR_LN_PLAY_P)||
					((u8_vtr_mode&STTR_HN_MASK)!=STTR_HN_S_PLAY))
			{
				// VTR dropped out from playback mode for some reason.
				go_to_error(ERR_CTRL_FAIL);
			}
			// Check user input.
			else if((u8_inputs&LINP_CAM_RR)==0)
			{
				// RR button released.
				// Cancel RR.
				go_to_switch_to_record();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Blink tally light fast in preparation.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					u8_outputs |= OUT_CAM_LED;
				}
			}
		}
		else if(u8_link_state==LST_PB_REW)
		{
			// Playback backwards (picture search/record review).
			load_serial_cmd(SCMD_REVIEW);
			// Check if state needs to be changed.
			// Check mechanical mode.
			if(((u8_vtr_mode&STTR_LN_MASK)!=STTR_LN_PLAY_P)&&
				((u8_vtr_mode&STTR_LN_MASK)!=STTR_LN_REVIEW))
			{
				// VTR dropped out from playback mode for some reason.
				go_to_error(ERR_CTRL_FAIL);
			}
			// Check user input.
			else if((u8_inputs&LINP_CAM_RR)==0)
			{
				// RR button released.
				// Switch from playback to record.
				go_to_play_pause_hold();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
			}
		}
		else if(u8_link_state==LST_PB_HOLD)
		{
			// Paused playback before returning to recording.
			load_serial_cmd(SCMD_STILL);
			// Check if state needs to be changed.
			// Check how much time is already spent in this holding mode.
			if(u8_ser_mode_dly==0)
			{
				// Timer run out.
				// Finally, switch to paused record.
				go_to_switch_to_record();
			}
			// Check user input.
			else if((u8_inputs&LINP_CAM_RR)!=0)
			{
				// User pressed button again, revert to backwards playback.
				u8_link_state = LST_PB_REW;
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Blink tally light fast in preparation.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					u8_outputs |= OUT_CAM_LED;
				}
			}
		}
		else if(u8_link_state==LST_SW_REC)
		{
			// Start transition from playback+pause to recording+pause.
			// Switch VTR to record.
			load_serial_cmd(SCMD_REC);
			// Check if state needs to be changed.
			// Check how much time is already spent trying to switch mode.
			if(u8_ser_mode_dly==0)
			{
				// Timer run out.
				// VTR took too long to switch to record mode, something is up.
				go_to_error(ERR_MODE_TIMEOUT);
			}
			// Check if VTR settled in record+pause.
			else if(((u8_vtr_mode&STTR_LN_MASK)==STTR_LN_REC_P)&&
					((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_RECP))
			{
				// Progress to the "record+pause" state.
				go_to_rec_paused();
			}
			else
			{
				// Enable pause for VTR and turn off tally light.
				u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
				// Blink tally light fast in preparation.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					u8_outputs |= OUT_CAM_LED;
				}
			}
		}
		else if(u8_link_state==LST_ERROR)
		{
			// Errored out of recording mode.
			// (control of VTR switched from camera to something else,
			// End Of Tape, some other reason)
			// Emergency stop VTR.
			load_serial_cmd(SCMD_STOP);
			// Check if state needs to be changed.
			// Check for error mode timeout.
			if(u8_ser_mode_dly==0)
			{
				// Time is up, can exit error now.
				// Check user input.
				if((u8_state&STATE_REC_LOCK)==0)
				{
					// Clear error to stop mode.
					u8_link_state = LST_STOP;
#ifndef MCU_LOW_ROM
					u8_ser_error = ERR_ALL_OK;
#endif	/* MCU_LOW_ROM */
				}
			}
			else
			{
				// Set lock if cleared during delay.
				u8_state |= STATE_REC_LOCK;
			}
			// Enable pause for VTR and turn off tally light.
			u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED);
			// Blink tally light slowly to indicate error state.
			if((u8_tasks&TASK_SLOW_BLINK)!=0)
			{
				u8_outputs |= OUT_CAM_LED;
			}
		}
		else
		{
			// Impossible state.
			go_to_error(ERR_LOGIC_FAIL);
		}
	}
	else
#endif	/* EN_SERIAL */
	{
		// No serial link detected, operating in wired ("dumb") mode.
#ifdef EN_SERIAL	
		// Check if previously operated in serial linked mode and link dropped out.
		if(u8_link_state!=LST_STOP)
		{
			// Reset out all serial link related variables.
#ifndef MCU_LOW_ROM
			u8_ser_error = ERR_LOST_LINK;
#endif	/* MCU_LOW_ROM */
			u8_link_state = LST_STOP;
			u8_ser_mode_dly = 0;
			u8_vtr_mode = 0;
			u8_ser_byte_cnt = u8_ser_bit_cnt = 0;
			// Indicate end of the transmission.
			DBG_3_OFF;
			// Clear record lock.
			u8_state &= ~STATE_REC_LOCK;
		}
#endif	/* EN_SERIAL */
		// Process events from lowest priority to highest priority.
		// Check video direction managed by VTR.
		if((u8_inputs&LINP_VTR_PB)!=0)
		{
			// VTR switched into playback mode.
			// Turn on pause and turn off standby commands to VTR.
			u8_outputs &= ~(OUT_VTR_RUN|OUT_VTR_STBY);
			// Switch video path from VTR to camera.
			u8_outputs |= (OUT_RLY_ON|OUT_CAM_PB);
			// (Attempt to) Clear record lock.
			u8_state &= ~STATE_REC_LOCK;
		}
		else
		{
			// VTR exited playback mode.
			// Switch video path from camera to VTR.
			u8_outputs &= ~(OUT_RLY_ON|OUT_CAM_PB);
		}
		// Check if camera commanded unpaused record.
		if((u8_state&STATE_REC_LOCK)!=0)
		{
			// Record mode commanded from camera.
			// Light up camera's tally light and unpause recording on VTR.
			u8_outputs |= (OUT_CAM_LED|OUT_VTR_RUN);
			// Check battery level.
			if((u8_state&STATE_LOW_BATT)!=0)
			{
				// Low battery condition detected.
				if(u8_rec_fade_dly==0)
				{
					// Possible fader must be done now.
					// Blink LED on the camera (mostly on, since recording is on).
					if((u8_tasks&TASK_BATT_BLINK)!=0)
					{
						u8_outputs &= ~OUT_CAM_LED;
					}
				}
			}
		}
		else
		{
			// Record mode cleared from camera.
			u8_outputs &= ~(OUT_CAM_LED|OUT_VTR_RUN);
			// Check battery level.
			if((u8_state&STATE_LOW_BATT)!=0)
			{
				// Low battery condition detected.
				if(u8_rec_fade_dly==0)
				{
					// Possible fader must be done now.
					// Blink LED on the camera (mostly off, since recording is off).
					if((u8_tasks&TASK_BATT_BLINK)!=0)
					{
						u8_outputs |= OUT_CAM_LED;
					}
				}
			}
		}
		// Check camera connection/power save state.
		if((u8_state&STATE_CAM_OFF)!=0)
		{
			// Camera is disconnected or in power save.
			// Cancel camera record lock.
			u8_state &= ~STATE_REC_LOCK;
			// Clear any record/playback mode.
			// (turn pause on, turn off camera's tally light, turn off video VTR to camera notifier)
			u8_outputs &= ~(OUT_VTR_RUN|OUT_CAM_LED|OUT_CAM_PB);
			// Put VTR into power save.
			u8_outputs |= OUT_VTR_STBY;
		}
		else
		{
			// Return VTR from power save.
			u8_outputs &= ~OUT_VTR_STBY;
		}
	}
}

//-------------------------------------- Apply all values to outputs.
static inline void apply_outputs(void)
{
	if((u8_outputs&OUT_RLY_ON)!=0)
	{
		RLY_ON;
	}
	else
	{
		RLY_OFF;
	}
	if((u8_outputs&OUT_VTR_RUN)!=0)
	{
		VTR_REC_RUN;
	}
	else
	{
		VTR_REC_PAUSE;
	}
#ifdef EN_WIRED_STANDBY
	if((u8_outputs&OUT_VTR_STBY)!=0)
	{
		VTR_STBY_ON;
	}
	else
#endif /* EN_WIRED_STANDBY */
	{
		VTR_STBY_OFF;
	}
	if((u8_outputs&OUT_CAM_LED)!=0)
	{
		CAM_LED_ON;
	}
	else
	{
		CAM_LED_OFF;
	}
	if((u8_outputs&OUT_CAM_PB)!=0)
	{
		CAM_VID_PB;
	}
	else
	{
		CAM_VID_REC;
	}
}

//====================================== MAIN LOOP.
int main(void)
{
	// Start-up initialization.
	system_startup();
	// Clear ADC history.
	for(uint8_t idx=0;idx<ADC_HIST_LEN;idx++)
	{
		u8a_12v_hist[idx] = 0;
		u8a_camv_hist[idx] = 0;
	}

	// Let hardware stabilize before reading anything.
	u8_start_dly = TIME_STARTUP;

#ifdef EN_SERIAL
	// Load stop command to be recognized by the VTR.	
	//load_serial_cmd(SCMD_STOP);
#endif	/* EN_SERIAL */
	
	// Main cycle.
    while (1) 
    {
		// Disable interrupts globally.
		cli();
		// Buffer all interrupts.
		u8_buf_interrupts |= u8i_interrupts;
		// Clear all interrupt flags (don't clear serial link presence flag).
		u8i_interrupts &= INTR_SERIAL;
		// Enable interrupts globally.
		sei();
		
		// Transfer serial link flag to the state.
		u8_state &= ~STATE_SERIAL_DET;
#ifdef EN_SERIAL
		if((u8_buf_interrupts&INTR_SERIAL)!=0)
		{
			u8_buf_interrupts &= ~INTR_SERIAL;
			u8_state |= STATE_SERIAL_DET;
		}
		if((u8_buf_interrupts&INTR_RX)!=0)
		{
			// Packet of data received from NV-180 VTR.
			u8_buf_interrupts &= ~INTR_RX;
			// Pick current VTR mode data.
			u8_vtr_mode = u8a_ser_data[SER_MN2UPD_OFS];
		}
#endif	/* EN_SERIAL */

		// Process deferred tasks.
		if((u8_buf_interrupts&INTR_SYS_TICK)!=0)
		{
			u8_buf_interrupts &= ~INTR_SYS_TICK;
			// System timing: 1000 Hz, 1000 us period.
			// Process additional timers.
			soft_timer_management();
			
			// Start ADC conversion.
			ADC_START;
			
			//DBG_PWM = u8_adc_12v;
			//DBG_PWM = u8_volt_12v;
			DBG_PWM = u8_cam_pwr;
			
			//if((u8_inputs&LINP_VTR_PB)==0)
			//if(u8_vid_dir_dly!=0)
			//if((u8_state&STATE_REC_LOCK)!=0)
			//if((u8_state&STATE_LOW_BATT)!=0)
#ifndef MCU_LOW_ROM
#ifdef EN_SERIAL
			// Check if serial link is established.
			if((u8_state&STATE_SERIAL_DET)==0)
			{
				// No serial link, operating in "dumb" mode.
				// Check pause output state.
				if((u8_outputs&OUT_VTR_RUN)==0)
				{
					DBG_1_ON;
				}
				else
				{
					DBG_1_OFF;
				}
			}
			// Serial link present, check VTR's record state.
			else if((u8_vtr_mode&STTR_HN_MASK)==STTR_HN_S_REC)
			{
				DBG_1_ON;
			}
			// Serial link present, VTR not recording,
			// check errors within serial control.
			else if((u8_ser_error!=ERR_ALL_OK)&&
					(u8_ser_error>u8_blink_cnt))
			{
				// Blink error code.
				if((u8_tasks&TASK_FAST_BLINK)!=0)
				{
					DBG_1_ON;
				}
				else
				{
					DBG_1_OFF;
				}
			}
			else
			{
				DBG_1_OFF;
			}
#else
			if((u8_outputs&OUT_VTR_RUN)==0)
			{
				DBG_1_ON;
			}
			else
			{
				DBG_1_OFF;
			}
#endif	/* EN_SERIAL */			
#endif	/* MCU_LOW_ROM */
			//if((u8_outputs&OUT_VTR_RUN)==0)
			//if(u8_rec_trg_dly!=0)
			//if((u8_state&STATE_SERIAL_DET)!=0)
			/*{
				DBG_2_ON;
			}
			else
			{
				DBG_2_OFF;
			}*/
			
			//if((u8_state&STATE_REC_LOCK)!=0)
			/*{
				DBG_3_ON;
			}
			else
			{
				DBG_3_OFF;
			}*/
			
			// Process slow events.
			if((u8_tasks&TASK_2HZ)!=0)
			{
				u8_tasks&=~TASK_2HZ;
				// 2 Hz event, 500 ms period.
				if((u8_state&STATE_SERIAL_DET)==0)
				{
					// Slow heartbeat: FW alive, but no serial link.
					DBG_2_TGL;
				}
				// Reset watchdog timer.
				wdt_reset();
				// Manage timing of certain states.
				slow_state_timing();
			}
			if((u8_tasks&TASK_5HZ_PH1)!=0)
			{
				u8_tasks&=~TASK_5HZ_PH1;
				// 5 Hz event (phase 1), 200 ms period.
				if((u8_state&STATE_SERIAL_DET)!=0)
				{
					// Fast heartbeat: FW alive, serial link is OK.
					DBG_2_TGL;
				}
				// Filter ADC data from 12 V input channel.
				filter_adc_ph1();
			}
			if((u8_tasks&TASK_5HZ_PH2)!=0)
			{
				u8_tasks&=~TASK_5HZ_PH2;
				// 5 Hz event (phase 2), 200 ms period.
				// Filter ADC data from camera power channel.
				filter_adc_ph2();
				// Check state of incoming power.
				input_power_check();
				// Calculate camera power consumption.
				camera_power_check();
			}
			if((u8_tasks&TASK_50HZ)!=0)
			{
				u8_tasks&=~TASK_50HZ;
				// 50 Hz event, 20 ms period.
				// Perform ADC history update.
				buffer_adc();
				// Read logic inputs.
				read_inputs();
				// Process inputs and produce outputs.
				state_machine();
				// Apply calculated values to outputs.
				apply_outputs();
			}
			if((u8_tasks&TASK_500HZ)!=0)
			{
				u8_tasks&=~TASK_500HZ;
				// 500 Hz event, 2 ms period.
				// Count-down various delays.
				delay_management();
			}
		}
    }
}
