/*
 * $Id: charresize.c,v 1.5 2000/12/12 11:18:14 kazuhiko Exp $
 *
 * Copyright 1988, 1992 Hiroto Kagotani
 * Copyright 2000 Kazuhiko
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	"def.h"

static void	processAttributes(void);
static void	startchar(void);
static void	encoding(void);
static void	swidth(void);
static void	dwidth(void);
static void	bbx(void);
static void	bitmap(void);
static void	endchar(void);
static void	read_image(char *srcimage);
static int	get_byte(char *cp);
static void	magnify(register char *srcimage, register int *dstgray);
static void	countup_score(int *dstgray, int x, int y);
static void	write_image(register int *dstgray);

static int	bbw;
static int	newbbw;
static int	bbh;
static int	newbbh;
static int	graylevel;

static int	bit[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void
processChar(void)
{
  char	*malloc();
  char	*srcimage;
  int	*dstgray;

  startchar();
  encoding();
  swidth();
  dwidth();
  bbx();
  processAttributes();
  bitmap();

  srcimage = (char*)malloc(bbw * bbh * sizeof(char));
  memset(srcimage, 0, bbw * bbh * sizeof(char));
  dstgray = (int*)malloc(newbbw * newbbh * sizeof(int));
  memset(dstgray, 0, newbbw * newbbh * sizeof(int));

  read_image(srcimage);
  magnify(srcimage, dstgray);
  write_image(dstgray);

  free(dstgray);
  free(srcimage);

  endchar();
}

static void
processAttributes(void)
{
  get_line();
  if (beginwith(linebuf, "ATTRIBUTES")) { /* optional */
    put_line();
    get_line();
  }
}

static void
startchar(void)
{
  get_line();
  if (!beginwith(linebuf, "STARTCHAR")) {
    error("STARTCHAR expected\n");
  }
  put_line();
}

static void
encoding(void)
{
  get_line();
  if (!beginwith(linebuf, "ENCODING")) {
    error("ENCODING expected\n");
  }
  put_line();
}

static void
swidth(void)
{
  int	arg1, arg2;
  int	newwx, newwy;

  get_line();
  if (!beginwith(linebuf, "SWIDTH")) {
    error("SWIDTH expected\n");
  }
  if (sscanf(linebuf, "SWIDTH %d %d", &arg1, &arg2) != 2) {
    error("SWIDTH has illegal arguments\n");
  }
  newwx = WRESIZE(arg1);
  newwy = HRESIZE(arg2);
  sprintf(linebuf, "SWIDTH %d %d", newwx, newwy);
  put_line();
}

static void
dwidth(void)
{
  int	arg1, arg2;
  int	newwx, newwy;

  get_line();
  if (!beginwith(linebuf, "DWIDTH")) {
    error("DWIDTH expected\n");
  }
  if (sscanf(linebuf, "DWIDTH %d %d", &arg1, &arg2) != 2) {
    error("DWIDTH has illegal arguments\n");
  }
  newwx = WRESIZE(arg1);
  newwy = HRESIZE(arg2);
  sprintf(linebuf, "DWIDTH %d %d", newwx, newwy);
  put_line();
}

static void
bbx(void)
{
  int	arg1, arg2, arg3, arg4;
  int	newbbox, newbboy;

  get_line();
  if (!beginwith(linebuf, "BBX")) {
    error("BBX expected\n");
  }
  if (sscanf(linebuf, "BBX %d %d %d %d", &arg1, &arg2, &arg3, &arg4) != 4
      || arg1 < 0 || arg2 < 0) {
    error("BBX has illegal arguments\n");
  }
  bbw = arg1;
  newbbw = WRESIZE(arg1);
  bbh = arg2;
  newbbh = HRESIZE(arg2);
  graylevel = bbw * bbh / blackness;

  newbbox = XRESIZE(arg3);
  newbboy = YRESIZE(arg4);
  sprintf(linebuf, "BBX %d %d %d %d", newbbw, newbbh, newbbox, newbboy);
  put_line();
}

static void
bitmap(void)
{
  if (!beginwith(linebuf, "BITMAP")) {
    error("BITMAP expected\n");
  }
  put_line();
}

static void
endchar(void)
{
  get_line();
  if (!beginwith(linebuf, "ENDCHAR")) {
    error("ENDCHAR expected\n");
  }
  put_line();
}

static void
read_image(char *srcimage)
{
  int	x, y, i;
  int	byte;

  for (y = 0; y < bbh; y ++) {
    get_line();
    for (x = 0; x < bbw; x += 8) {
      byte = get_byte(linebuf+x/4);
      for (i = 0; i < 8 && x+i < bbw; i ++) {
	*srcimage++ = ((byte & bit[i]) != 0);
      }
    }
  }
}

static int
get_byte(char *cp)
{
  int	byte = 0;
  if (!isxdigit((int)*cp)) {
    error("illegal raster data\n");
  }
  byte |= xdigit(*cp);
  byte <<= 4;
  cp ++;
  if (!isxdigit((int)*cp)) {
    error("illegal raster data\n");
  }
  byte |= xdigit(*cp);
  return byte;
}

static void
magnify(register char *srcimage, register int *dstgray)
{
  register int	x, y;

  for (y = 0; y < bbh; y ++) {
    for (x = 0; x < bbw; x ++) {
      if (srcimage[y*bbw + x]) {
	countup_score(dstgray, x, y);
      }
    }
  }
}

static void
countup_score(int *dstgray, int	x, int	y)
{
  int	X = x * newbbw;
  int	Y = y * newbbh;
  int	X1 = X + newbbw;
  int	Y1 = Y + newbbh;

  int	newx, newy;
  int	newxbegin = X / bbw;
  int	newybegin = Y / bbh;
  int	newxend = howmany(X1, bbw);
  int	newyend = howmany(Y1, bbh);

  for (newy = newybegin; newy < newyend; newy ++) {
    for (newx = newxbegin; newx < newxend; newx ++) {
      int	newX = newx * bbw;
      int	newY = newy * bbh;
      int	newX1 = newX + bbw;
      int	newY1 = newY + bbh;
      dstgray[newy * newbbw + newx] +=
	(MIN(X1,newX1) - MAX(X,newX))
	* (MIN(Y1,newY1) - MAX(Y,newY));
    }
  }
}

static void
write_image(register int *dstgray)
{
  register int	x, y, i;
  for (y = 0; y < newbbh; y ++) {
    register int	*row = dstgray + y*newbbw;
    for (x = 0; x < newbbw; x += 8) {
      register int	byte = 0;
      for (i = 0; i < 8 && x+i < newbbw; i ++) {
	if (row[x + i] >= graylevel) {
	  byte |= bit[i];
	}
      }
      printf("%02X", byte);
    }
    printf("\n");
  }
}
