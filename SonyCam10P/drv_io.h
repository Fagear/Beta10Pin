/**************************************************************************************************************************************************************
drv_io.h

Copyright � 2024 Maksim Kryukov <fagear@mail.ru>

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

// Video direction relay control output.
#define RLY_PORT		PORTB
#define RLY_DIR			DDRB
#define RLY_BIT			(1<<0)
#define RLY_ON			(RLY_PORT|=RLY_BIT)
#define RLY_OFF			(RLY_PORT&=~RLY_BIT)

// NV-180 serial link.
#define VTR_SER_PORT	PORTD
#define VTR_SER_DIR		DDRD
#define VTR_SER_SRC		PIND
#define VTR_SCLK_BIT	(1<<1)
#define VTR_SDAT_BIT	(1<<0)
#define VTR_SCLK_STATE	(VTR_SER_SRC&VTR_SCLK_BIT)
#define VTR_SDAT_STATE	(VTR_SER_SRC&VTR_SDAT_BIT)
// Video signal direction input from 10-pin VTR connector.
#define VTR_VID_PORT	PORTB
#define VTR_VID_DIR		DDRB
#define VTR_VID_SRC		PINB
#define VTR_VID_BIT		(1<<1)
#define VTR_VID_STATE	(VTR_VID_SRC&VTR_VID_BIT)
#define VTR_VID_PB		(VTR_VID_STATE!=0)
#define VTR_VID_REC		(VTR_VID_STATE==0)
// Standby output for NV-180 VTR.
#define VTR_STBY_PORT	PORTD
#define VTR_STBY_DIR	DDRD
#define VTR_STBY_BIT	(1<<3)
#define VTR_STBY_ON		(VTR_STBY_PORT|=VTR_STBY_BIT)
#define VTR_STBY_OFF	(VTR_STBY_PORT&=~VTR_STBY_BIT)
// Rec/play output for 10-pin VTR connector.
#define VTR_REC_PORT	PORTD
#define VTR_REC_DIR		DDRD
#define VTR_REC_BIT		(1<<4)
#define VTR_REC_RUN		(VTR_REC_PORT|=VTR_REC_BIT)
#define VTR_REC_PAUSE	(VTR_REC_PORT&=~VTR_REC_BIT)

// Camera record button input.
#define CAM_REC_PORT	PORTD
#define CAM_REC_DIR		DDRD
#define CAM_REC_SRC		PIND
#define CAM_REC_PIN		(1<<6)
#define CAM_REC_STATE	(CAM_REC_SRC&CAM_REC_PIN)
#define CAM_REC_HIGH	(CAM_REC_STATE!=0)
#define CAM_REC_LOW		(CAM_REC_STATE==0)
// Camera record/review button input.
#define CAM_RR_PORT		PORTD
#define CAM_RR_DIR		DDRD
#define CAM_RR_SRC		PIND
#define CAM_RR_PIN		(1<<5)
#define CAM_RR_STATE	(CAM_RR_SRC&CAM_RR_PIN)
#define CAM_RR_UP		(CAM_RR_STATE!=0)
#define CAM_RR_DOWN		(CAM_RR_STATE==0)
// Camera record/battery light (tally) output.
#define CAM_LED_PORT	PORTD
#define CAM_LED_DIR		DDRD
#define CAM_LED_PIN		(1<<7)
#define CAM_LED_ON		(CAM_LED_PORT|=CAM_LED_PIN)
#define CAM_LED_OFF		(CAM_LED_PORT&=~CAM_LED_PIN)
#define CAM_LED_TGL		(CAM_LED_PORT^=CAM_LED_PIN)
// Camera video selector output.
#define CAM_VID_PORT	PORTD
#define CAM_VID_DIR		DDRD
#define CAM_VID_PIN		(1<<2)
#define CAM_VID_PB		(CAM_VID_PORT|=CAM_VID_PIN)
#define CAM_VID_REC		(CAM_VID_PORT&=~CAM_VID_PIN)

// Power supply ADC inputs.
#define ADC_INT			ADC_vect
#define ADC_CONFIG1		ADMUX = (0<<REFS0)|(0<<REFS1)|(1<<ADLAR)|(0<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3)
#define ADC_CONFIG2		ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS0)|(0<<ADPS1)|(1<<ADPS2)
#define ADC_CONFIG3		DIDR0 |= ((1<<ADC0D)|(1<<ADC1D))
#define ADC_START		ADCSRA |= (1<<ADSC)
#define ADC_MUX_RD		(ADMUX&((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3)))
#define ADC_MUX_CLR		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3))
#define ADC_MUX_WR		ADMUX
#define ADC_CH_12V		((0<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3))
#define ADC_CH_CAM		((1<<MUX0)|(0<<MUX1)|(0<<MUX2)|(0<<MUX3))
#define ADC_DATA		ADCH
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
#define WDT_SW_ON			WDTCSR=(1<<WDE)|(1<<WDP0)|(1<<WDP1)|(1<<WDP2)	// MCU reset after ~2.0 s

// System timer setup.
#define SYST_INT			TIMER2_COMPA_vect			// Interrupt vector alias
#define SYST_CONFIG1		TCCR2A=(1<<WGM21)			// CTC mode (clear on compare with OCR)
#define SYST_CONFIG2		OCR2A=124					// Cycle clock: INclk/(1+124), 1000 Hz cycle
#define SYST_EN_INTR		TIMSK2|=(1<<OCIE2A)			// Enable interrupt
#define SYST_DIS_INTR		TIMSK2&=~(1<<OCIE2A)		// Disable interrupt
#define SYST_START			TCCR2B|=(1<<CS21)			// Start timer with clk/8 clock (125 kHz)
#define SYST_STOP			TCCR2B&=~((1<<CS20)|(1<<CS21)|(1<<CS22))	// Stop timer
#define SYST_DATA_8			TCNT2						// Count register
#define SYST_RESET			SYST_DATA_8=0				// Reset count

// Power consumption optimizations.
#define PWR_COMP_OFF		ACSR|=(1<<ACD)
#define PWR_ADC_OFF			PRR|=(1<<PRADC)
#define PWR_T0_OFF			PRR|=(1<<PRTIM0)
#define PWR_T1_OFF			PRR|=(1<<PRTIM1)
#define PWR_T2_OFF			PRR|=(1<<PRTIM2)
#define PWR_I2C_OFF			PRR|=(1<<PRTWI)
#define PWR_SPI_OFF			PRR|=(1<<PRSPI)
#define PWR_UART_OFF		PRR|=(1<<PRUSART0)

//-------------------------------------- IO initialization.
inline void HW_init(void)
{
	// Init outputs.
	RLY_PORT &= ~RLY_BIT;			RLY_DIR |= RLY_BIT;				// Video switching relay control.
	VTR_REC_PORT &= ~VTR_REC_BIT;	VTR_REC_DIR |= VTR_REC_BIT;		// VTR record pause control.
	VTR_STBY_PORT &= ~VTR_STBY_BIT;	VTR_STBY_DIR |= VTR_STBY_BIT;	// VTR standby control.
	CAM_LED_PORT &= ~CAM_LED_PIN;	CAM_LED_DIR |= CAM_LED_PIN;		// Camera record/battery indicator control.
	CAM_VID_PORT &= ~CAM_VID_PIN;	CAM_VID_DIR |= CAM_VID_PIN;		// Camera viewfinder video select control.
	
	// Init inputs.
	CAM_REC_PORT |= CAM_REC_PIN;	CAM_REC_DIR &= ~CAM_REC_PIN;	// Camera record button input.
	CAM_RR_PORT |= CAM_RR_PIN;		CAM_RR_DIR &= ~CAM_RR_PIN;		// Camera RR button input.
	VTR_VID_PORT |= VTR_VID_BIT;	VTR_VID_DIR &= ~VTR_VID_BIT;	// Video signal direction input.
	VTR_SER_PORT |= (VTR_SDAT_BIT|VTR_SCLK_BIT);	VTR_SER_DIR &= ~(VTR_SDAT_BIT|VTR_SCLK_BIT);	// Serial link pins.
	ADC_PORT &= ~(ADC_12V_PIN|ADC_CAM_PIN);			ADC_DIR &= ~(ADC_12V_PIN|ADC_CAM_PIN);			// ADC power supply measurement inputs.
	
	// System timing.
	SYST_CONFIG1;
	SYST_CONFIG2;
	SYST_RESET;
	SYST_EN_INTR;
	
	// Debug PWM.
	TCCR2A = (1<<COM2A1)|(1<<WGM21)|(1<<WGM20);
	TCCR2B = (1<<CS20);
	TCNT2 = 0;
	OCR2A = 0;
	DDRB |= (1<<3);
	DDRB |= (1<<4);
	
	// ADC configuration.
	ADC_CONFIG1; ADC_CONFIG2; ADC_CONFIG3;
	
	// Turn off unused modules for power saving.
	PWR_COMP_OFF; PWR_I2C_OFF; PWR_SPI_OFF; PWR_UART_OFF; PWR_T0_OFF; PWR_T1_OFF;
}

#endif /* DRV_IO_H_ */