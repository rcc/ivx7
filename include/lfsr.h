/*
 * lfsr.h - lfsr library
 *
 * Copyright (C) 2009 Robert C. Curtis
 *
 * lfsr.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * lfsr.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lfsr.h. If not, see <http://www.gnu.org/licenses/>.
 */

/****************************************************************************/

#include <stdint.h>

#ifndef I__LFSR_H__
	#define I__LFSR_H__

extern const uint8_t lfsr_taps4[];
extern const uint8_t lfsr_taps8[];
extern const uint16_t lfsr_taps12[];
extern const uint16_t lfsr_taps16[];
extern const uint32_t lfsr_taps20[];
extern const uint32_t lfsr_taps24[];
extern const uint32_t lfsr_taps28[];
extern const uint32_t lfsr_taps32[];

uint8_t lfsr_inc_8(const uint8_t *taps, uint8_t *lfsr);
uint16_t lfsr_inc_16(const uint16_t *taps, uint16_t *lfsr);
uint32_t lfsr_inc_32(const uint32_t *taps, uint32_t *lfsr);

#endif /* I__LFSR_H__ */
