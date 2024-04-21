#include "sony10p.h"

volatile uint8_t u8i_interrupts = 0;		// Deferred interrupts call flags (non-buffered)
uint8_t u8_buf_interrupts = 0;				// Deferred interrupts call flags (buffered)
uint8_t u8_tasks=0;							// Deferred tasks call flags
uint8_t u8_500hz_cnt=0;						// Divider for 500 Hz
uint8_t u8_50hz_cnt=0;						// Divider for 50 Hz
uint8_t u8_10hz_cnt=0;						// Divider for 10 Hz
uint8_t u8_2hz_cnt=0;						// Divider for 2 Hz
uint8_t u8_adc_12v=0;						// Voltage level at the power input
uint8_t u8_adc_cam=0;						// Voltage level at the camera output
uint8_t u8_cam_pwr=0;						// Camera consumption

// Firmware description strings.
volatile const uint8_t ucaf_version[] PROGMEM = "v0.01";			// Firmware version
volatile const uint8_t ucaf_compile_time[] PROGMEM = __TIME__;		// Time of compilation
volatile const uint8_t ucaf_compile_date[] PROGMEM = __DATE__;		// Date of compilation
volatile const uint8_t ucaf_info[] PROGMEM = "Sony Beta camera 14-pin to 10-pin EIAJ adapter";	// Firmware description
volatile const uint8_t ucaf_author[] PROGMEM = "Maksim Kryukov aka Fagear (fagear@mail.ru)";	// Author

//-------------------------------------- System timer interrupt handler.
ISR(SYST_INT, ISR_NAKED)
{
	INTR_IN;
	//PORTB |= (1<<4);
	// 1000 Hz event.
	u8i_interrupts |= INTR_SYS_TICK;
	//PORTB &= ~(1<<4);
	INTR_OUT;
}

//-------------------------------------- ADC interrupt handler.
ISR(ADC_INT)
{
	u8i_interrupts |= INTR_READ_ADC;
	uint8_t mux;
	//PORTB |= (1<<4);
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
	//PORTB &= ~(1<<4);
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
			u8_tasks |= TASK_50HZ;

			u8_10hz_cnt++;
			if(u8_10hz_cnt>=5)	// 50/5 = 10.
			{
				u8_10hz_cnt = 0;
				// 10 Hz event.
				u8_tasks |= TASK_10HZ;

				u8_2hz_cnt++;
				if(u8_2hz_cnt>=5)	// 10/5 = 2.
				{
					u8_2hz_cnt = 0;
					// 2 Hz event.
					u8_tasks |= TASK_2HZ;
				}
			}
		}
	}
}

int main(void)
{
	// Start-up initialization.
	system_startup();

	// Enable interrupts globally.
	sei();

	// Main cycle.
    while (1) 
    {
		// Disable interrupts globally.
		cli();
		// Buffer all interrupts.
		u8_buf_interrupts |= u8i_interrupts;
		// Clear all interrupt flags.
		u8i_interrupts=0;
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
			
			OCR2A = u8_adc_cam;
							
			if(VTR_VID_PB)
			{
				CAM_VID_PB;
				RLY_ON;
				CAM_LED_OFF;
			}
			else
			{
				CAM_VID_REC;
				RLY_OFF;
				CAM_LED_ON;
			}
			
			if(CAM_REC_LOW)
			{
				VTR_REC_RUN;
			}
			else
			{
				VTR_REC_PAUSE;
			}
			
			// Process slow events.
			if((u8_tasks&TASK_2HZ)!=0)
			{
				u8_tasks&=~TASK_2HZ;
				// 2 Hz event, 500 ms period.
				// Toggle slow blink flag.
				u8_tasks^=TASK_SLOW_BLINK;
				// Reset watchdog timer.
				wdt_reset();
			}
			if((u8_tasks&TASK_10HZ)!=0)
			{
				u8_tasks&=~TASK_10HZ;
				// 10 Hz event.
				// Toggle fast blink flag.
				u8_tasks^=TASK_FAST_BLINK;
				// Reset watchdog timer.
			}
			if((u8_tasks&TASK_500HZ)!=0)
			{
				u8_tasks&=~TASK_500HZ;
				// 500 Hz event.
				
			}
		}
    }
}
