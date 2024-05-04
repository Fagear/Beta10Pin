/**************************************************************************************************************************************************************
drv_io.h

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

Hardware defines (pseudo-HAL) and setup routines.

**************************************************************************************************************************************************************/

#ifndef DRV_IO_H_
#define DRV_IO_H_

#include <avr/io.h>

// Debugging:
// ADC_12V - PC0 (in)
// ADC_CAM - PC1 (in)

// VID_SW - PB0 (out)

// CAM_LIGHT - PD7 (out)
// CAM_VID - PD2 (out)
// CAM_REC - PD6 (in)
// CAM_RR - PD5 (in)

// VTR_VID - PB1 (in)
// VTR_REC - PD4 (out)
// VTR_STBY - PD3 (out)
// VTR_SCLK - PD1 (in)
// VTR_SDAT - PD0 (in/out)

// Production PCB:
// ADC_12V - PC0 (in)
// ADC_CAM - PC1 (in)

// VID_SW - PB1 (out)

// CAM_LIGHT - PD3 (out)
// CAM_VID - PD2 (out)
// CAM_REC - PD4 (in)
// CAM_RR - PD0 (in)

// VTR_VID - PB0 (in)
// VTR_REC - PD5 (out)
// VTR_STBY - PD6 (out)
// VTR_SCLK - PD7 (in)
// VTR_SDAT - PD1 (in/out)

// Video direction relay control output.
#define RLY_PORT		PORTB
#define RLY_DIR			DDRB
#define RLY_BIT			(1<<1)
#define RLY_ON			(RLY_PORT|=RLY_BIT)
#define RLY_OFF			(RLY_PORT&=~RLY_BIT)

// NV-180 serial link.
#define VTR_SER_INT		PCINT2_vect						// Interrupt vector alias
#define VTR_SER_PORT	PORTD
#define VTR_SER_DIR		DDRD
#define VTR_SER_SRC		PIND
#define VTR_SCLK_BIT	(1<<7)
#define VTR_SDAT_BIT	(1<<1)
#define VTR_SCLK_STATE	(VTR_SER_SRC&VTR_SCLK_BIT)
#define VTR_SDAT_STATE	(VTR_SER_SRC&VTR_SDAT_BIT)
#define VTR_SDAT_IN		(VTR_SER_DIR&=~VTR_SDAT_BIT)
#define VTR_SDAT_OUT	(VTR_SER_DIR|=VTR_SDAT_BIT)
#define VTR_SDAT_SET0	(VTR_SER_PORT&=~VTR_SDAT_BIT)
#define VTR_SDAT_SET1	(VTR_SER_PORT|=VTR_SDAT_BIT)
#define VTR_SER_CONFIG1	(PCICR|=(1<<PCIE2))
#define VTR_SER_CONFIG2	(PCMSK2|=(1<<PCINT23))
// Video signal direction input from 10-pin VTR connector.
#define VTR_VID_PORT	PORTB
#define VTR_VID_DIR		DDRB
#define VTR_VID_SRC		PINB
#define VTR_VID_BIT		(1<<0)
#define VTR_VID_STATE	(VTR_VID_SRC&VTR_VID_BIT)
#define VTR_VID_PB		(VTR_VID_STATE!=0)
#define VTR_VID_REC		(VTR_VID_STATE==0)
// Standby output for NV-180 VTR.
#define VTR_STBY_PORT	PORTD
#define VTR_STBY_DIR	DDRD
#define VTR_STBY_BIT	(1<<6)
#define VTR_STBY_ON		(VTR_STBY_PORT|=VTR_STBY_BIT)
#define VTR_STBY_OFF	(VTR_STBY_PORT&=~VTR_STBY_BIT)
// Rec/play output for 10-pin VTR connector.
#define VTR_REC_PORT	PORTD
#define VTR_REC_DIR		DDRD
#define VTR_REC_BIT		(1<<5)
#define VTR_REC_RUN		(VTR_REC_PORT|=VTR_REC_BIT)
#define VTR_REC_PAUSE	(VTR_REC_PORT&=~VTR_REC_BIT)

// Camera record button input.
#define CAM_REC_PORT	PORTD
#define CAM_REC_DIR		DDRD
#define CAM_REC_SRC		PIND
#define CAM_REC_PIN		(1<<4)
#define CAM_REC_STATE	(CAM_REC_SRC&CAM_REC_PIN)
#define CAM_REC_HIGH	(CAM_REC_STATE!=0)
#define CAM_REC_LOW		(CAM_REC_STATE==0)
// Camera record/review button input.
#define CAM_RR_PORT		PORTD
#define CAM_RR_DIR		DDRD
#define CAM_RR_SRC		PIND
#define CAM_RR_PIN		(1<<0)
#define CAM_RR_STATE	(CAM_RR_SRC&CAM_RR_PIN)
#define CAM_RR_UP		(CAM_RR_STATE!=0)
#define CAM_RR_DOWN		(CAM_RR_STATE==0)
// Camera record/battery light (tally) output.
#define CAM_LED_PORT	PORTD
#define CAM_LED_DIR		DDRD
#define CAM_LED_PIN		(1<<3)
#define CAM_LED_ON		(CAM_LED_PORT|=CAM_LED_PIN)
#define CAM_LED_OFF		(CAM_LED_PORT&=~CAM_LED_PIN)
#define CAM_LED_TGL		(CAM_LED_PORT^=CAM_LED_PIN)
// Camera video selector output.
#define CAM_VID_PORT	PORTD
#define CAM_VID_DIR		DDRD
#define CAM_VID_PIN		(1<<2)
#define CAM_VID_PB		(CAM_VID_PORT|=CAM_VID_PIN)
#define CAM_VID_REC		(CAM_VID_PORT&=~CAM_VID_PIN)

// Debug outputs.
#define DBG_PORT		PORTB
#define DBG_DIR			DDRB
#define DBG_1_PIN		(1<<5)
#define DBG_2_PIN		(1<<4)
#define DBG_3_PIN		(1<<3)
#define DBG_4_PIN		(1<<2)
#define DBG_1_ON		(DBG_PORT|=DBG_1_PIN)
#define DBG_1_OFF		(DBG_PORT&=~DBG_1_PIN)
#define DBG_1_TGL		(DBG_PORT^=DBG_1_PIN)
#define DBG_2_ON		(DBG_PORT|=DBG_2_PIN)
#define DBG_2_OFF		(DBG_PORT&=~DBG_2_PIN)
#define DBG_2_TGL		(DBG_PORT^=DBG_2_PIN)
#define DBG_3_ON		(DBG_PORT|=DBG_3_PIN)
#define DBG_3_OFF		(DBG_PORT&=~DBG_3_PIN)
#define DBG_3_TGL		(DBG_PORT^=DBG_3_PIN)
#define DBG_PWM			OCR1B

// Power supply ADC inputs.
#define ADC_INT			ADC_vect						// Interrupt vector alias
#define ADC_CONFIG1		ADMUX = (0<<REFS0)|(0<<REFS1)|(0<<ADLAR)|(0<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3)
#define ADC_CONFIG2		ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS0)|(0<<ADPS1)|(1<<ADPS2)
#define ADC_PWR_SAVE	DIDR0 |= ((1<<ADC0D)|(1<<ADC1D))
#define ADC_START		ADCSRA |= (1<<ADSC)
#define ADC_MUX_RD		(ADMUX&((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3)))
#define ADC_MUX_CLR		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3))
#define ADC_MUX_WR		ADMUX
#define ADC_CH_12V		((0<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3))
#define ADC_CH_CAM		((1<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3))
#define ADC_DATA		ADC
#define ADC_PORT		PORTC
#define ADC_DIR			DDRC
#define ADC_12V_PIN		(1<<0)
#define ADC_CAM_PIN		(1<<1)

// Watchdog setup.
#define WDT_RESET_DIS		MCUSR&=~(1<<WDRF)
#define WDT_PREP_OFF		WDTCSR|=(1<<WDCE)|(1<<WDE)
#define WDT_SW_OFF			WDTCSR=0x00
#define WDT_FLUSH_REASON	MCUSR=(0<<WDRF)|(0<<BORF)|(0<<EXTRF)|(0<<PORF)
#define WDT_PREP_ON			WDTCSR|=(1<<WDCE)|(1<<WDE)
#define WDT_SW_ON			WDTCSR=(1<<WDE)|(0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0)	// MCU reset after ~1.0 s

// System timer setup.
#define SYST_INT			TIMER2_COMPA_vect			// Interrupt vector alias
#define SYST_CONFIG1		TCCR2A=(1<<WGM21)			// CTC mode (clear on compare with OCR)
#define SYST_CONFIG2		TCCR2B=0		
#define SYST_CONFIG3		OCR2A=124					// Cycle clock: clk/(1+124), 1000 Hz cycle
#define SYST_EN_INTR		TIMSK2|=(1<<OCIE2A)			// Enable interrupt
#define SYST_DIS_INTR		TIMSK2&=~(1<<OCIE2A)		// Disable interrupt
#define SYST_START			TCCR2B|=(1<<CS22)			// Start timer with clk/64 clock (125 kHz)
#define SYST_STOP			TCCR2B&=~((1<<CS20)|(1<<CS21)|(1<<CS22))	// Stop timer
#define SYST_DATA_8			TCNT2						// Count register
#define SYST_RESET			SYST_DATA_8=0				// Reset count

// Serial timer setup.
#define SERT_INT			TIMER0_OVF_vect				// Interrupt vector alias
#define SERT_CONFIG1		TCCR0A=0					// Normal mode
#define SERT_CONFIG2		TCCR0B=0
#define SERT_EN_INTR		TIMSK0|=(1<<TOIE0)			// Enable interrupt
#define SERT_START			TCCR0B|=((1<<CS01)|(1<<CS00))				// Start timer with clk/64 clock (125 kHz)
#define SERT_STOP			TCCR0B&=~((1<<CS00)|(1<<CS01)|(1<<CS02))	// Stop timer
#define SERT_DATA_8			TCNT0						// Count register
#define SERT_RESET			SERT_DATA_8=0				// Reset count

// Power consumption optimizations.
#define PWR_COMP_OFF		ACSR|=(1<<ACD)
#define PWR_SAVE			PRR
#define PWR_ADC_OFF			(1<<PRADC)
#define PWR_T0_OFF			(1<<PRTIM0)
#define PWR_T1_OFF			(1<<PRTIM1)
#define PWR_T2_OFF			(1<<PRTIM2)
#define PWR_I2C_OFF			(1<<PRTWI)
#define PWR_SPI_OFF			(1<<PRSPI)
#define PWR_UART_OFF		(1<<PRUSART0)

//-------------------------------------- IO initialization.
inline void HW_init(void)
{
	// Init outputs.
	RLY_PORT &= ~RLY_BIT;			RLY_DIR |= RLY_BIT;				// Video switching relay control.
	VTR_REC_PORT &= ~VTR_REC_BIT;	VTR_REC_DIR |= VTR_REC_BIT;		// VTR record pause control.
#ifdef EN_STANDBY
	VTR_STBY_PORT &= ~VTR_STBY_BIT;	VTR_STBY_DIR |= VTR_STBY_BIT;	// VTR standby control.
#else
	VTR_STBY_PORT &= ~VTR_STBY_BIT;	VTR_STBY_DIR &= ~VTR_STBY_BIT;	// Disabled standby control.
#endif /* EN_STANDBY */
	CAM_LED_PORT &= ~CAM_LED_PIN;	CAM_LED_DIR |= CAM_LED_PIN;		// Camera record/battery indicator control.
	CAM_VID_PORT &= ~CAM_VID_PIN;	CAM_VID_DIR |= CAM_VID_PIN;		// Camera viewfinder video select control.
	
	// Init inputs.
	CAM_REC_PORT |= CAM_REC_PIN;	CAM_REC_DIR &= ~CAM_REC_PIN;	// Camera record button input.
	CAM_RR_PORT |= CAM_RR_PIN;		CAM_RR_DIR &= ~CAM_RR_PIN;		// Camera RR button input.
	VTR_VID_PORT |= VTR_VID_BIT;	VTR_VID_DIR &= ~VTR_VID_BIT;	// Video signal direction input.
#ifdef EN_SERIAL
	VTR_SER_PORT |= VTR_SDAT_BIT;	VTR_SER_DIR &= ~VTR_SDAT_BIT;	// Serial link data pin.
	VTR_SER_PORT |= VTR_SCLK_BIT;	VTR_SER_DIR &= ~VTR_SCLK_BIT;	// Serial link clock pin input.
#else
	VTR_SER_PORT &= ~VTR_SDAT_BIT;	VTR_SER_DIR &= ~VTR_SDAT_BIT;
	VTR_SER_PORT &= ~VTR_SCLK_BIT;	VTR_SER_DIR &= ~VTR_SCLK_BIT;
#endif	/* EN_SERIAL */
	ADC_PORT &= ~ADC_12V_PIN;		ADC_DIR &= ~ADC_12V_PIN;		// ADC input power supply measurement pin.
	ADC_PORT &= ~ADC_CAM_PIN;		ADC_DIR &= ~ADC_CAM_PIN;		// ADC camera supply measurement pin.
	
	// System timing.
	SYST_CONFIG1; SYST_CONFIG2; SYST_CONFIG3;
	SYST_RESET;
	SYST_EN_INTR;

#ifdef EN_SERIAL	
	// Serial link timing.
	SERT_CONFIG1; SERT_CONFIG2;
	SERT_RESET;
	SERT_EN_INTR;

	// Serial link interrupts.
	VTR_SER_CONFIG1; VTR_SER_CONFIG2;
#endif	/* EN_SERIAL */
	
	// ADC configuration.
	ADC_CONFIG1; ADC_CONFIG2;
	ADC_PWR_SAVE;
	
	// Debug outputs.
	DBG_PORT &= ~(DBG_1_PIN|DBG_2_PIN|DBG_3_PIN);
	DBG_DIR |= (DBG_1_PIN|DBG_2_PIN|DBG_3_PIN);

	// Debug PWM.
	DBG_PORT &= ~DBG_4_PIN;		DBG_DIR |= DBG_4_PIN;				// OC1B output
	TCNT1 = 0;
	OCR1B = 0;
	TCCR1A = (1<<COM1B1)|(1<<WGM10);								// Fast non-inverting 8-bit PWM
	TCCR1B = (1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);				// Start timer with clk/1 clock (8 MHz), 31.25 kHz cycle
	
	// Turn off unused modules for power saving.
	PWR_COMP_OFF;
	PWR_SAVE |= (PWR_I2C_OFF|PWR_SPI_OFF|PWR_UART_OFF);
}

#endif /* DRV_IO_H_ */
