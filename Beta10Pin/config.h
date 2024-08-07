/**************************************************************************************************************************************************************
config.h

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

Created: 2021-04-13

Part of the [Beta10Pin] project.
Pre-compile configuration file.
Defines/switches for configuring compile-time options.

**************************************************************************************************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

// Allow wired standby operations
#define EN_WIRED_STANDBY

// Allow camera power detection
#define EN_CAM_PWR_DEF

// Allow operation over Panasonic serial link
#define EN_SERIAL

// Get size of the ROM in the MCU
#ifdef FLASHSTART
	#define MCU_ROM_SIZE	(FLASHEND - FLASHSTART)
#else
	#define MCU_ROM_SIZE	FLASHEND
#endif

// Check for 4K MCUs
#if MCU_ROM_SIZE<0x1000
	// For 4K MCUs error code blinking is not available
	#error Low flash ROM, MCU not supported!
#endif

#endif /* CONFIG_H_ */
