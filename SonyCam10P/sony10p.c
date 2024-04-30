#include "sony10p.h"

volatile uint8_t u8i_interrupts = 0;		// Deferred interrupts call flags (non-buffered)
uint8_t u8_buf_interrupts = 0;				// Deferred interrupts call flags (buffered)
uint8_t u8_ser_byte_cnt=0;					// Byte count in the serial packed transmission
uint8_t u8_ser_bit_cnt=0;					// Bit count in the serial packed transmission
volatile uint8_t u8a_ser_data[SER_PACK_LEN];// Data storage for serial transmission
uint8_t u8_ser_cmd=0;						// Command to be sent through serial link to NV-180 VTR
uint8_t u8_tasks=0;							// Deferred tasks call flags
uint8_t u8_state=0;							// Camera and VTR state
uint8_t u8_500hz_cnt=0;						// Divider for 500 Hz
uint8_t u8_50hz_cnt=0;						// Divider for 50 Hz
uint8_t u8_5hz_cnt=0;						// Divider for 5 Hz
uint8_t u8_2hz_cnt=0;						// Divider for 2 Hz
uint8_t u8_adc_12v=0;						// Voltage level at the power input
uint8_t u8_adc_cam=0;						// Voltage level at the camera output
uint8_t u8_cam_pwr=0;						// Camera consumption (voltage difference)
uint8_t u8_inputs=0;						// Inputs state storage
uint8_t u8_outputs=0;						// Outputs state storage
uint8_t u8_start_dly=0;						// Start-up delay before processing anything
uint8_t u8_vid_dir_dly=0;					// Delay after detected video direction change
uint8_t u8_rec_trg_dly=0;					// Delay for detecting triggered record input
uint8_t u8_rr_dly=0;						// Delay between switching state between review/pause
uint8_t u8_ser_cmd_dly=0;					// Duration for sending new command to the VTR
uint8_t u8_pause_time=0;					// Timer for record pause to standby.
uint8_t u8_idle_time=0;						// Timer for stop to standby.

// LUT for 10-bit to 8-bit conversion to pick 9.0...15.0 V range.
const uint8_t ucaf_adc_to_byte[1024] PROGMEM =
{
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 1,	 1,	 2,	 2,
	3,	 4,	 4,	 5,	 5,	 6,	 7,	 7,
	8,	 8,	 9,	 10,	 10,	 11,	 11,	 12,
	13,	 13,	 14,	 14,	 15,	 16,	 16,	 17,
	18,	 18,	 19,	 19,	 20,	 21,	 21,	 22,
	22,	 23,	 24,	 24,	 25,	 25,	 26,	 27,
	27,	 28,	 28,	 29,	 30,	 30,	 31,	 31,
	32,	 33,	 33,	 34,	 34,	 35,	 36,	 36,
	37,	 37,	 38,	 39,	 39,	 40,	 40,	 41,
	42,	 42,	 43,	 43,	 44,	 45,	 45,	 46,
	46,	 47,	 48,	 48,	 49,	 49,	 50,	 51,
	51,	 52,	 53,	 53,	 54,	 54,	 55,	 56,
	56,	 57,	 57,	 58,	 59,	 59,	 60,	 60,
	61,	 62,	 62,	 63,	 63,	 64,	 65,	 65,
	66,	 66,	 67,	 68,	 68,	 69,	 69,	 70,
	71,	 71,	 72,	 72,	 73,	 74,	 74,	 75,
	75,	 76,	 77,	 77,	 78,	 78,	 79,	 80,
	80,	 81,	 81,	 82,	 83,	 83,	 84,	 84,
	85,	 86,	 86,	 87,	 88,	 88,	 89,	 89,
	90,	 91,	 91,	 92,	 92,	 93,	 94,	 94,
	95,	 95,	 96,	 97,	 97,	 98,	 98,	 99,
	100,	 100,	 101,	 101,	 102,	 103,	 103,	 104,
	104,	 105,	 106,	 106,	 107,	 107,	 108,	 109,
	109,	 110,	 110,	 111,	 112,	 112,	 113,	 113,
	114,	 115,	 115,	 116,	 116,	 117,	 118,	 118,
	119,	 119,	 120,	 121,	 121,	 122,	 123,	 123,
	124,	 124,	 125,	 126,	 126,	 127,	 127,	 128,
	129,	 129,	 130,	 130,	 131,	 132,	 132,	 133,
	133,	 134,	 135,	 135,	 136,	 136,	 137,	 138,
	138,	 139,	 139,	 140,	 141,	 141,	 142,	 142,
	143,	 144,	 144,	 145,	 145,	 146,	 147,	 147,
	148,	 148,	 149,	 150,	 150,	 151,	 151,	 152,
	153,	 153,	 154,	 154,	 155,	 156,	 156,	 157,
	158,	 158,	 159,	 159,	 160,	 161,	 161,	 162,
	162,	 163,	 164,	 164,	 165,	 165,	 166,	 167,
	167,	 168,	 168,	 169,	 170,	 170,	 171,	 171,
	172,	 173,	 173,	 174,	 174,	 175,	 176,	 176,
	177,	 177,	 178,	 179,	 179,	 180,	 180,	 181,
	182,	 182,	 183,	 183,	 184,	 185,	 185,	 186,
	186,	 187,	 188,	 188,	 189,	 189,	 190,	 191,
	191,	 192,	 193,	 193,	 194,	 194,	 195,	 196,
	196,	 197,	 197,	 198,	 199,	 199,	 200,	 200,
	201,	 202,	 202,	 203,	 203,	 204,	 205,	 205,
	206,	 206,	 207,	 208,	 208,	 209,	 209,	 210,
	211,	 211,	 212,	 212,	 213,	 214,	 214,	 215,
	215,	 216,	 217,	 217,	 218,	 218,	 219,	 220,
	220,	 221,	 221,	 222,	 223,	 223,	 224,	 225,
	225,	 226,	 226,	 227,	 228,	 228,	 229,	 229,
	230,	 231,	 231,	 232,	 232,	 233,	 234,	 234,
	235,	 235,	 236,	 237,	 237,	 238,	 238,	 239,
	240,	 240,	 241,	 241,	 242,	 243,	 243,	 244,
	244,	 245,	 246,	 246,	 247,	 247,	 248,	 249,
	249,	 250,	 250,	 251,	 252,	 252,	 253,	 253
};



// Firmware description strings.
volatile const uint8_t ucaf_version[] PROGMEM = "v0.03";			// Firmware version
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
	//uint16_t adc_data;
	//adc_data = ADC_DATA;
	//data = (adc_data>>2)&0xFF;
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

//-------------------------------------- Serial link pin change interrupt handler.
ISR(VTR_SER_INT)
{
	uint8_t timer_data;
	// Save timer count.
	timer_data = SERT_DATA_8;
	// Clear and restart timer.
	SERT_RESET; SERT_START;
	// Check for rising or falling edge.
	if(VTR_SCLK_STATE==0)
	{
		// Start of a new pulse.
		if(timer_data<TIME_SER_CLK)
		{
			// Pulse within single-byte transmission.
			if(u8_ser_bit_cnt>0)
			{
				// Switch to the next bit.
				u8_ser_bit_cnt--;
			}
			// By default, byte offset [SER_CAM2VTR_OFS] is used for TX from camera, all other byte for RX.
			if(u8_ser_byte_cnt==SER_CAM2VTR_OFS)
			{
				// Preset next data bit for output to be read at rising edge by VTR.
				if((u8a_ser_data[SER_CAM2VTR_OFS]&(1<<u8_ser_bit_cnt))==0)
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
			if(u8_ser_byte_cnt<SER_PACK_LEN)
			{
				// Switch to the next byte.
				u8_ser_byte_cnt++;
				// Clear next byte in the buffer.
				u8a_ser_data[u8_ser_byte_cnt] = 0;
				// Start from the bit 7.
				u8_ser_bit_cnt = SER_LAST_BIT;
			}
		}
		else
		{
			// Start of the new packed transmission after a pause.
			DBG_1_ON;
			// Reset bit and byte counters.
			u8_ser_byte_cnt = 0;
			u8_ser_bit_cnt = SER_LAST_BIT;
			// Check if serial link was detected on the first transmission from VTR.
			if((u8i_interrupts&INTR_SERIAL)!=0)
			{
				// Preset data bit 7 for output to be read at rising edge by VTR.
				if((u8a_ser_data[SER_CAM2VTR_OFS]&(1<<u8_ser_bit_cnt))==0)
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
			DBG_1_OFF;
		}
	}
	else
	{
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
					u8a_ser_data[u8_ser_byte_cnt] |= (1<<u8_ser_bit_cnt);
				}
			}
		}
		else
		{
			// Tally signal.
			u8i_interrupts |= INTR_TALLY;
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
	// Check if transmission took place and finished ok.
	if(u8_ser_byte_cnt==(SER_PACK_LEN-1))
	{
		// Lock in presence of serial link.
		u8i_interrupts |= INTR_SERIAL;
		// Load a command into the buffer.
		u8a_ser_data[SER_CAM2VTR_OFS] = u8_ser_cmd;
	}
	else
	{
		// Serial link in not established.
		u8i_interrupts &= ~INTR_SERIAL;
	}
}

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

	// Enable watchdog (reset in ~4 s).
	WDT_PREP_ON;
	WDT_SW_ON;
	wdt_reset();
}

//-------------------------------------- Slow events dividers.
static inline void slow_timing(void)
{
	u8_500hz_cnt++;
	if(u8_500hz_cnt>=2)		// 1000/2 = 500
	{
		u8_500hz_cnt = 0;
		// 500 Hz event.
		u8_tasks |= TASK_500HZ;

		u8_50hz_cnt++;
		if(u8_50hz_cnt>=10)		// 500/10 = 50.
		{
			u8_50hz_cnt = 0;
			// 50 Hz event.

			u8_5hz_cnt++;
			if(u8_5hz_cnt>=2)
			{
				// Turn off fast blink 20% duty cycle.
				u8_tasks &= ~TASK_FAST_BLINK;
			}
			if(u8_5hz_cnt>=10)	// 50/10 = 5.
			{
				u8_5hz_cnt = 0;
				// 5 Hz event.
				u8_tasks |= TASK_5HZ;
				// Turn on fast blink.
				u8_tasks |= TASK_FAST_BLINK;
			}
			u8_2hz_cnt++;
			if(u8_2hz_cnt>=5)
			{
				// Turn off slow blink 20% duty cycle.
				u8_tasks &= ~TASK_SLOW_BLINK;
			}
			if(u8_2hz_cnt>=25)	// 50/25 = 2.
			{
				u8_2hz_cnt = 0;
				// 2 Hz event.
				u8_tasks |= TASK_2HZ;
				// Turn on slow blink.
				u8_tasks |= TASK_SLOW_BLINK;
			}
		}
	}
}

//-------------------------------------- Check state of input power supply.
static inline void input_power_check(void)
{
	u8_state &= ~STATE_LOW_BATT;
	// Don't check if input voltage channel is not read yet.
	if(u8_adc_12v==0)
	{
		return;
	}
	// Check if "low battery" indication should be lit.
	if(u8_adc_12v<VIN_LOW_BATT)
	{
		u8_state |= STATE_LOW_BATT;
	}
}

//-------------------------------------- Determine camera power consumption.
static inline void camera_power_check(void)
{
	u8_cam_pwr = 0;
	// Don't calculate consumption if one of the ADC channels is not read yet.
	if((u8_adc_12v==0)||(u8_adc_cam==0))
	{
		return;
	}
	// Prevent overflow if voltage dividers/ADC tolerances lead to impossible.
	if(u8_adc_12v<u8_adc_cam)
	{
		return;
	}
	// Calculate camera consumption.
	u8_cam_pwr = u8_adc_12v - u8_adc_cam;
	if(u8_cam_pwr<32)
	{
		u8_cam_pwr = (u8_cam_pwr*8);
	}
	else
	{
		u8_cam_pwr = 255;
	}
	// Check if camera is powered on.
	u8_state &= ~STATE_CAM_OFF;
	if(u8_cam_pwr<VD_CAM_ON)
	{
		u8_state |= STATE_CAM_OFF;
	}
}

//-------------------------------------- Process timeouts.
static inline void delay_management(void)
{
	// Count down every 2 ms.
	if(u8_start_dly!=0) u8_start_dly--;
	if(u8_vid_dir_dly!=0) u8_vid_dir_dly--;
	if(u8_rec_trg_dly!=0) u8_rec_trg_dly--;
	if(u8_rr_dly!=0) u8_rr_dly--;
	if(u8_ser_cmd_dly!=0)
	{
		u8_ser_cmd_dly--;
	}
	else
	{
		// Expire command, go idle.
		u8_ser_cmd = SCMD_IDLE;
	}
}

//-------------------------------------- Process standby timers.
static inline void standby_management(void)
{
	// Check if serial link is present.
	if((u8_state&STATE_SERIAL_DET)!=0)
	{
		// Standby control is relevant only for NV-180 that has serial link.
		if(u8_pause_time!=0)
		{
			u8_pause_time--;
			if(u8_pause_time==0)
			{
				u8_tasks |= TASK_TIME_STBY;
			}
		}
		if(u8_idle_time!=0)
		{
			u8_idle_time--;
			if(u8_idle_time==0)
			{
				u8_tasks |= TASK_TIME_STBY;
			}
		}
	}
	else
	{
		u8_pause_time = u8_idle_time = 0;
		u8_tasks &= ~TASK_TIME_STBY;
	}
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
			// Previous signal state was low.
			// Update to a new state.
			u8_inputs |= LINP_CAM_REC;
			// Reset timer to catch triggered input.
			u8_rec_trg_dly = TIME_CAM_REC;
			// Check if record command wasn't locked on previous falling edge.
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
			// Previous signal state was high.
			// Update to a new state.
			u8_inputs &= ~LINP_CAM_REC;
			// Check if trigger lock run out.
			if(u8_rec_trg_dly==0)
			{
				// Triggered record signal is not detected.
				// Treat input signal is direct and pause decoding.
				// Stop "record" period.
				u8_state &= ~STATE_REC_LOCK;
			}
		}
	}
	if((u8_state&STATE_REC_LOCK)==0)
	{
		// Allow record review only in no-record state.
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
	else
	{
		// Clear RR condition.
		u8_inputs &= ~LINP_CAM_RR;
	}
}

//-------------------------------------- Upload new command to the VTR.
void load_serial_cmd(uint8_t new_cmd)
{
	// Buffer new command.
	u8_ser_cmd = new_cmd;
	// Set timer for duration.
	u8_ser_cmd_dly = TIME_CMD;
}

//-------------------------------------- Process inputs and generate outputs.
static inline void state_machine(void)
{
	// Process events from lowest priority to highest priority.
	if((u8_inputs&LINP_CAM_RR)!=0)
	{
		// Check if serial link is present.
		if((u8_state&STATE_SERIAL_DET)!=0)
		{
			// Review mode commanded from camera.
			// Send "review" command to the VTR through serial link.
			load_serial_cmd(SCMD_REVIEW);
		}
	}
	if((u8_inputs&LINP_VTR_PB)!=0)
	{
		// VTR switched into playback mode.
		u8_outputs &= ~(OUT_VTR_RUN|OUT_VTR_STBY);
		u8_outputs |= (OUT_RLY_ON|OUT_CAM_PB);
		// Clear record lock.
		u8_state &= ~STATE_REC_LOCK;
	}
	else
	{
		// VTR exited playback mode.
		u8_outputs &= ~(OUT_RLY_ON|OUT_CAM_PB);
	}
	if((u8_state&STATE_REC_LOCK)!=0)
	{
		// Record mode commanded from camera.
		u8_outputs |= (OUT_CAM_LED|OUT_VTR_RUN);
		// Check if serial link is present.
		if((u8_state&STATE_SERIAL_DET)!=0)
		{
			// Send "record" command to the VTR through serial link.
			load_serial_cmd(SCMD_REC);
		}
		if((u8_state&STATE_LOW_BATT)!=0)
		{
			// Low battery condition detected.
			if((u8_tasks&TASK_SLOW_BLINK)!=0)
			{
				// Blink LED on the camera (mostly on).
				u8_outputs &= ~OUT_CAM_LED;
			}
		}
	}
	else
	{
		// Record mode cleared from camera.
		u8_outputs &= ~(OUT_CAM_LED|OUT_VTR_RUN);
		if((u8_state&STATE_LOW_BATT)!=0)
		{
			// Low battery condition detected.
			if((u8_tasks&TASK_SLOW_BLINK)!=0)
			{
				// Blink LED on the camera (mostly off).
				u8_outputs |= OUT_CAM_LED;
			}
		}
	}
	if((u8_state&STATE_CAM_OFF)!=0)
	{
		// Camera is disconnected or in power save.
		// Cancel camera record lock.
		u8_state &= ~STATE_REC_LOCK;
		// Clear any record/playback mode.
		u8_outputs &= ~(OUT_RLY_ON|OUT_VTR_RUN|OUT_CAM_LED|OUT_CAM_PB);
		// Check if serial link is present.
		//if((u8_state&STATE_SERIAL_DET)!=0) // TODO
		{
			// Put VTR into power save.
			u8_outputs |= OUT_VTR_STBY;
		}
	}
	else
	{
		// Return VTR from power save.
		u8_outputs &= ~OUT_VTR_STBY;
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
	if((u8_outputs&OUT_VTR_STBY)!=0)
	{
		VTR_STBY_ON;
	}
	else
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
	uint8_t last_mode = 0;
	
	// Start-up initialization.
	system_startup();

	// Let hardware stabilize before reading anything.
	u8_start_dly = TIME_STARTUP;

	// Load stop command to be recognized by the VTR.	
	load_serial_cmd(SCMD_STOP);
	
	// Main cycle.
    while (1) 
    {
		// Disable interrupts globally.
		cli();
		// Buffer all interrupts.
		u8_buf_interrupts |= u8i_interrupts;
		// Clear all interrupt flags (don't clear serial link presence flag).
		u8i_interrupts = 0;
		// Enable interrupts globally.
		sei();
		
		if(last_mode!=u8a_ser_data[SER_MN2UPD_OFS])
		{
			//DBG_2_ON;
			last_mode = u8a_ser_data[SER_MN2UPD_OFS];
			//DBG_2_OFF;
		}
		
		// Transfer serial link flag to the state.
		u8_state &= ~STATE_SERIAL_DET;
		if((u8_buf_interrupts&INTR_SERIAL)!=0)
		{
			u8_state |= STATE_SERIAL_DET;
		}
		// Process deferred tasks.
		if((u8_buf_interrupts&INTR_SYS_TICK)!=0)
		{
			u8_buf_interrupts &= ~INTR_SYS_TICK;
			// System timing: 1000 Hz, 1000 us period.
			// Process additional slow timers.
			slow_timing();
			
			// Start ADC conversion.
			ADC_START;
			
			// Check state of incoming power.
			input_power_check();
			// Calculate camera power consumption.
			camera_power_check();
			// Read logic inputs.
			read_inputs();
			// Process inputs and produce outputs.
			state_machine();
			// Apply calculated values to outputs.
			apply_outputs();
			
			//DBG_PWM = u8_cam_pwr;
			DBG_PWM = u8_cam_pwr;
			//if((u8_inputs&LINP_VTR_PB)==0)
			//if(u8_vid_dir_dly!=0)
			if((u8_state&STATE_CAM_OFF)!=0)
			{
				DBG_2_ON;
			}
			else
			{
				DBG_2_OFF;
			}
			
			//if((u8_outputs&OUT_VTR_RUN)==0)
			if(u8_ser_cmd_dly!=0)
			{
				DBG_3_ON;
			}
			else
			{
				DBG_3_OFF;
			}
			
			// Process slow events.
			if((u8_tasks&TASK_2HZ)!=0)
			{
				u8_tasks&=~TASK_2HZ;
				// 2 Hz event, 500 ms period.
				//DBG_3_TGL;
				// Reset watchdog timer.
				wdt_reset();
				standby_management();
			}
			if((u8_tasks&TASK_500HZ)!=0)
			{
				u8_tasks&=~TASK_500HZ;
				// 500 Hz event, 2 ms period.
				delay_management();
			}
		}
    }
}
