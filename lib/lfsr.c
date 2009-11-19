/*
 * lfsr.c - lfsr library
 *
 * Copyright (C) 2009 Robert C. Curtis
 *
 * lfsr.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * lfsr.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lfsr.c. If not, see <http://www.gnu.org/licenses/>.
 */

/****************************************************************************/

#include <lfsr.h>
#include <prjutil.h>

/* first element is the LFSR mask, then its a NULL terminated list of taps */
const uint8_t lfsr_taps4[] =
	{0xF, (1 << 3), (1 << 2), 0};
const uint8_t lfsr_taps8[] =
	{0xFF, (1 << 7), (1 << 5), (1 << 4), (1 << 3), 0};
const uint16_t lfsr_taps12[] =
	{0xFFF, (1 << 11), (1 << 5), (1 << 3), (1 << 0), 0};
const uint16_t lfsr_taps16[] =
	{0xFFFF, (1 << 15), (1 << 14), (1 << 12), (1 << 3), 0};
const uint32_t lfsr_taps20[] =
	{0xFFFFF, (1 << 19), (1 << 16), 0};
const uint32_t lfsr_taps24[] =
	{0xFFFFFF, (1 << 23), (1 << 22), (1 << 21), (1 << 16), 0};
const uint32_t lfsr_taps28[] =
	{0xFFFFFFF, (1 << 27), (1 << 24), 0};
const uint32_t lfsr_taps32[] =
	{0xFFFFFFFF, (1 << 31), (1 << 21), (1 << 1), (1 << 0), 0};

uint8_t lfsr_inc_8(const uint8_t *taps, uint8_t *lfsr)
{
	uint8_t tap = 0;
	int i = 1;

	while(taps[i])
		tap ^= !!(taps[i++] & *lfsr);
	*lfsr <<= 1;
	*lfsr |= tap;
	*lfsr &= taps[0];

	return *lfsr;
}

uint16_t lfsr_inc_16(const uint16_t *taps, uint16_t *lfsr)
{
	uint16_t tap = 0;
	int i = 1;

	while(taps[i])
		tap ^= !!(taps[i++] & *lfsr);
	*lfsr <<= 1;
	*lfsr |= tap;
	*lfsr &= taps[0];

	return *lfsr;
}

uint32_t lfsr_inc_32(const uint32_t *taps, uint32_t *lfsr)
{
	uint32_t tap = 0;
	int i = 1;

	while(taps[i])
		tap ^= !!(taps[i++] & *lfsr);
	*lfsr <<= 1;
	*lfsr |= tap;
	*lfsr &= taps[0];

	return *lfsr;
}
