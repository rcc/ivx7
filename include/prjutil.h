/*
 * prjutil.h - Commonly useful macros
 *
 * Copyright (C) 2009 Robert C. Curtis
 *
 * prjutil.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * prjutil.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with prjutil.h. If not, see <http://www.gnu.org/licenses/>.
 */

/****************************************************************************/

#ifndef I__PRJUTIL_H__
	#define I__PRJUTIL_H__

/*******************************************************************************
 * NULL
 * 	Global NULL definition
 */
#ifndef NULL
#define NULL ((void *)0)
#endif
/******************************************************************************/

/*******************************************************************************
 * offsetof
 * 	The offset of member with structure of type
 *
 * type:	the structure type
 * member:	the name of the member within the struct.
 */
#ifndef offsetof
#define offsetof(TYPE, MEMBER)	__builtin_offsetof(TYPE, MEMBER)
#endif
/******************************************************************************/

/*******************************************************************************
 * container_of
 * 	Cast a member of a structure out to the containing structure
 *
 * ptr:		the pointer to the member.
 * type:	the type of the container struct this is embedded in.
 * member:	the name of the member within the struct.
 */
#ifndef container_of
#define container_of(ptr, type, member)	({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#endif
/******************************************************************************/

/*******************************************************************************
 * num_elements
 * 	Get the number of elements in an array
 *
 * array:	array variable
 */
#ifndef num_elements
#define num_elements(array)	(sizeof((array))/sizeof((array)[0]))
#endif
/******************************************************************************/

/*******************************************************************************
 * max_of
 * 	Get the max of two values
 */
#ifndef max_of
#define max_of(x,y)		((y > x) ? y : x)
#endif
/******************************************************************************/

/*******************************************************************************
 * min_of
 * 	Get the min of two values
 */
#ifndef min_of
#define min_of(x,y)		((y < x) ? y : x)
#endif
/******************************************************************************/

/*******************************************************************************
 * pdebug
 * 	printf that only works with DEBUG_PRINTS
 */
#ifndef pdebug
#ifdef DEBUG_PRINTS
#define pdebug(fmt, args...) printf(fmt, ## args)
#else
#define pdebug(fmt, args...)
#endif
#endif
/******************************************************************************/

#endif /* I__PRJUTIL_H__ */
