/*
 * $Id: def.h,v 1.4 2000/12/12 01:32:24 kazuhiko Exp $
 *
 * Copyright 1988, 1992 Hiroto Kagotani
 * Copyright 2000 Kazuhiko
 * Copyright 2000 Masao Uebayashi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */ 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern int	numerator_x, denominator_x;
extern int	numerator_y, denominator_y;
extern int	blackness;
extern char	linebuf[];

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#ifndef howmany
# define howmany(x, y)  (((x)+((y)-1))/(y))
#endif

#define	xdigit(c)	((c)>='a' ? (c)-'a'+10\
					  : (c)>='A' ? (c)-'A'+10\
						     : (c)-'0')
#define	WRESIZE(a)	howmany((a) * numerator_x, denominator_x)
#define	HRESIZE(a)	howmany((a) * numerator_y, denominator_y)
#define	XRESIZE(a)	((a)>=0						\
		? ((a) * numerator_x * 2 / denominator_x + 1) / 2	\
		: -((-(a) * numerator_x * 2 / denominator_x + 1) / 2))
#define	YRESIZE(a)	((a)>=0						\
		? ((a) * numerator_y * 2 / denominator_y + 1) / 2	\
		: -((-(a) * numerator_y * 2 / denominator_y + 1) / 2))

#define	BUFSIZE		1024

/* bdfresize.c */
void	get_line(void);
void	put_line(void);
int	beginwith(char *l, char *s);
void	error(char *fmt);

/* charresize.c */
void	processChar(void);
