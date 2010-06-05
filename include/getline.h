/*
 * getline.h
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

#include <stdio.h>
#include <sys/types.h>

#ifndef I__GETLINE_H__
	#define I__GETLINE_H__

/* getline
 * 	fp	- file pointer
 * 	buf	- buffer to put line in
 * 	bufsz	- max size of buffer
 *
 * 	returns size of line, negative on EOF
 */
ssize_t getline(FILE *fp, char *buf, size_t bufsz);

#endif /* I__GETLINE_H__ */
