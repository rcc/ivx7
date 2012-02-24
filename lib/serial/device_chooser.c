/*
 * Copyright 2012 Robert C. Curtis. All rights reserved.
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

#include <stdio.h>
#include <string.h>

#include <logging.h>
#include <serial/serial.h>

int serial_device_chooser(char *devpath, size_t pathlen)
{
	char **devlist = serial_device_list();
	size_t i = 0;
	char choice = 'a';
	size_t count;

	if(devlist == NULL)
		return -1;

	/* List devices with choice letters */
	for(i = 0; devlist[i]; i++) {
		printf("Serial device (%c) : %s\n", choice, devlist[i]);
		choice++;
	}
	count = i;

	/* Prompt user */
	printf("Select a device by its letter: ");
	choice = getc(stdin);
	choice -= 'a';
	if(choice < 0 || choice >= count) {
		logerror("invalid device selection\n");
		serial_free_device_list(devlist);
		return -1;
	} else {
		strncpy(devpath, devlist[(size_t)choice], pathlen);
	}

	serial_free_device_list(devlist);

	return 0;
}
