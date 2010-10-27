/*
 * getline.c
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * getline is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * getline is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with getline.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <getline.h>

ssize_t fgetline(FILE *fp, char *buf, size_t bufsz)
{
	size_t i = 0;
	int c;

	while(((c = getc(fp)) != EOF) && (i < (bufsz - 1))) {
		if(((char)c != '\n') && ((char)c != '\r')) {
			buf[i++] = (char)c;
		} else {
			/* check for a matching pair */
			if((char)c == '\n') {
				if((char)(c = getc(fp)) != '\r')
					ungetc(c, fp);
			} else if((char)c == '\r') {
				if((char)(c = getc(fp)) != '\n')
					ungetc(c, fp);
			}
			break;
		}
	}

	buf[i] = '\0'; /* NULL Terminate */

	/* if its the end of the file, and there was no line, return -1 */
	if((c == EOF) && !i)
		return -1;

	return (ssize_t)i;
}
