/*
 * Copyright 2011 Robert C. Curtis. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBERT C. CURTIS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ROBERT C. CURTIS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Robert C. Curtis.
 */

#include <hexdump.h>
#include <prjutil.h>

static void hexdump_line(FILE *outf, void *line, size_t size, size_t width)
{
	size_t i;

	for(i = 0; i < width; i++) {
		if(i < size) {
			fprintf(outf, "%02x ",
					*((unsigned char *)(line + i)) & 0xFF);
		} else {
			fprintf(outf, "   ");
		}
	}
	fprintf(outf, " ");
	for(i = 0; i < width; i++) {
		char c = (i < size) ? *((char *)(line + i)) : ' ';
		if((c < 32) || (c > 126))
			c = '.';
		fprintf(outf, "%c", c);
	}
}

void hexdump(FILE *outf, void *buf, size_t size)
{
	size_t i;
	const char *addrfmt;

	if(size < (1 << 8)) {
		addrfmt = "%02x: ";
	} else if(size < (1 << 16)) {
		addrfmt = "%04x: ";
	} else {
		addrfmt = "%08x: ";
	}

	for(i = 0; i < size; i += HEXDUMP_WIDTH) {
		fprintf(outf, addrfmt, (unsigned int)i);
		hexdump_line(outf, buf + i, min_of(HEXDUMP_WIDTH, size - i),
				HEXDUMP_WIDTH);
		fprintf(outf, "\n");
	}
}
