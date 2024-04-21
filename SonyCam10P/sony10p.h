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

// Flags for [u8_tasks].
#define	TASK_500HZ			(1<<0)	// 500 Hz event
#define	TASK_50HZ			(1<<1)	// 50 Hz event
#define	TASK_10HZ			(1<<2)	// 10 Hz event
#define	TASK_2HZ			(1<<3)	// 2 Hz event
#define	TASK_SLOW_BLINK		(1<<4)	// Indicator slow blink source
#define	TASK_FAST_BLINK		(1<<5)	// Indicator fast blink source

#endif /* SONY10P_H_ */
