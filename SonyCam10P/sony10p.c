#include "sony10p.h"

volatile uint8_t u8i_interrupts = 0;		// Deferred interrupts call flags (non-buffered)
uint8_t u8_buf_interrupts = 0;				// Deferred interrupts call flags (buffered)
uint8_t u8_ser_byte_cnt=0;					// Byte count in the serial packed transmittion
uint8_t u8_ser_bit_cnt=0;					// Bit count in the serial packed transmittion
volatile uint8_t u8a_ser_data[SER_PACK_LEN];// Data storage for serial transmittion
uint8_t u8_tasks=0;							// Deferred tasks call flags
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

// Firmware description strings.
volatile const uint8_t ucaf_version[] PROGMEM = "v0.02";			// Firmware version
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
	uint8_t mux;
	// Read last mux state.
	mux = ADC_MUX_RD;
	// Clear selected channel.
	ADC_MUX_CLR;
	if(mux==ADC_CH_12V)
	{
		// Save input 12V voltage level.
		u8_adc_12v = ADC_DATA;
		// Switch to next ADC channel.
		ADC_MUX_WR |= ADC_CH_CAM;
	}
	else
	{
		// Save output 12V to camera voltage level.
		u8_adc_cam = ADC_DATA;
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
			// Pulse within single-byte transmittion.
			if(u8_ser_bit_cnt>0)
			{
				// Switch to the next bit.
				u8_ser_bit_cnt--;
			}
			if(u8_ser_byte_cnt==0)
			{
				if((u8a_ser_data[0]&(1<<u8_ser_bit_cnt))==0)
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
			// Start of the new byte (2nd+) in packed transmittion.
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
			// Start of the new packed transmittion after a pause.
			DBG_1_ON;
			// Reset bit and byte counters.
			u8_ser_byte_cnt = 0;
			u8_ser_bit_cnt = SER_LAST_BIT;
			// Check if serial link was detected on the first transmittion from VTR.
			if((u8i_interrupts&INTR_SERIAL_DET)!=0)
			{
				// Preset data bit 0 for output.
				if((u8a_ser_data[0]&(1<<u8_ser_bit_cnt))==0)
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
			// Pulse within single-byte transmittion.
			if((u8_ser_byte_cnt>0)&&(u8_ser_byte_cnt<SER_PACK_LEN))
			{
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
	// Check if transmittion took place and finished ok.
	if(u8_ser_byte_cnt==(SER_PACK_LEN-1))
	{
		// Lock in presence of serial link.
		u8i_interrupts |= INTR_SERIAL_DET;
	}
	else
	{
		u8i_interrupts &= ~INTR_SERIAL_DET;
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
	u8_tasks &= ~TASK_LOW_BATT;
	// Don't check if input voltage channel is not read yet.
	if(u8_adc_12v==0)
	{
		return;
	}
	// Check if "low battery" indication should be lit.
	if(u8_adc_12v<VIN_LOW_BATT)
	{
		u8_tasks |= TASK_LOW_BATT;
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
	// Prevent overflow if dividers/ADC tolerances lead to impossible.
	if(u8_adc_12v<u8_adc_cam)
	{
		return;
	}
	// Calculate camera consumption.
	u8_cam_pwr = u8_adc_12v - u8_adc_cam;
	//u8_cam_pwr = (u8_cam_pwr*4);
}

//-------------------------------------- Process timeouts.
static inline void delay_management(void)
{
	if(u8_start_dly!=0) u8_start_dly--;
	if(u8_vid_dir_dly!=0) u8_vid_dir_dly--;
	if(u8_rec_trg_dly!=0) u8_rec_trg_dly--;
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
			if((u8_inputs&LINP_REC_LOCK)==0)
			{
				// Start "record" period.
				u8_inputs |= LINP_REC_LOCK;
			}
			else
			{
				// Stop "record" period.
				u8_inputs &= ~LINP_REC_LOCK;
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
				u8_inputs &= ~LINP_REC_LOCK;
				// TODO: clear rec lock on camera standby
			}
		}
	}
	if(CAM_RR_DOWN)
	{
		// Camera record/review button signal is in low state.
		u8_inputs |= LINP_CAM_RR;
	}
	else
	{
		// Camera record/review button signal is in high state.
		u8_inputs &= ~LINP_CAM_RR;
	}
}

//-------------------------------------- Process inputs and generate outputs.
static inline void state_machine(void)
{
	if((u8_inputs&LINP_REC_LOCK)!=0)
	{
		// Record mode commanded from camera.
		u8_outputs |= (OUT_CAM_LED|OUT_VTR_RUN);
		if((u8_tasks&TASK_LOW_BATT)!=0)
		{
			// Low battery condition detected.
			if((u8_tasks&TASK_FAST_BLINK)!=0)
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
		if((u8_tasks&TASK_LOW_BATT)!=0)
		{
			// Low battery condition detected.
			if((u8_tasks&TASK_FAST_BLINK)!=0)
			{
				// Blink LED on the camera (mostly off).
				u8_outputs |= OUT_CAM_LED;
			}
		}
	}
	if((u8_inputs&LINP_VTR_PB)!=0)
	{
		// VTR switched into playback mode.
		u8_outputs &= ~(OUT_VTR_RUN|OUT_VTR_STBY);
		u8_outputs |= (OUT_RLY_ON|OUT_CAM_PB);
	}
	else
	{
		// VTR exited playback mode.
		u8_outputs &= ~(OUT_RLY_ON|OUT_CAM_PB);
	}
	if((u8_tasks&TASK_CAMERA_OFF)!=0)
	{
		// Camera is disconnected or in power save.
		// Cancel camera record lock.
		u8_inputs &= ~LINP_REC_LOCK;
		// Clear any record/playback mode.
		u8_outputs &= ~(OUT_RLY_ON|OUT_VTR_RUN|OUT_CAM_LED|OUT_CAM_PB);
		// Put VTR into power save.
		u8_outputs |= OUT_VTR_STBY;
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

int main(void)
{
	// Start-up initialization.
	system_startup();

	// Let hardware stabilize before reading anything.
	u8_start_dly = TIME_STARTUP;
	// Clear first byte in the buffer.
	u8a_ser_data[0] = SCMD_STOP;
	
	// Main cycle.
    while (1) 
    {
		// Disable interrupts globally.
		cli();
		// Buffer all interrupts.
		u8_buf_interrupts |= u8i_interrupts;
		// Clear all interrupt flags (don't clear serial link presence flag).
		u8i_interrupts &= INTR_SERIAL_DET;
		// Enable interrupts globally.
		sei();
		
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
			
			DBG_PWM = u8_cam_pwr;
			//if((u8_buf_interrupts&INTR_SERIAL_DET)!=0)
			//if((u8_inputs&LINP_VTR_PB)==0)
			if(u8_vid_dir_dly!=0)
			{
				DBG_2_ON;
			}
			else
			{
				DBG_2_OFF;
			}
			if((u8_outputs&OUT_VTR_RUN)==0)
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
				// Reset watchdog timer.
				wdt_reset();
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
