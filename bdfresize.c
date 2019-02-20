/*
 * $Id: bdfresize.c,v 1.5 2000/12/12 11:18:14 kazuhiko Exp $
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	"def.h"
#include	"getopt.h"

int main(int argc, char *argv[]);
static int	processHeader(void);
static void	processFooter(void);
static void	processProperties(void);
static void	wresize(char *prop);
static void	hresize(char *prop);
static void	yresize(char *prop);
static void	startfont(void);
static void	font(void);
static void	resize_xlfd(char *xlfd);
static int	isnum(char *p);
static void	size(void);
static void	fontboundingbox(void);
static int	chars(void);
static void	endfont(void);
static void	usage(void);
static void	version(void);

int	numerator_x = 1, denominator_x = 1;
int	numerator_y = 1, denominator_y = 1;
int	blackness = 4;
char	linebuf[BUFSIZE];

static int	line;

int
main(int argc, char *argv[])
{
  int	c, err = 0;
  int	charcount;
  while ((c = getopt(argc, argv, "vb:w:h:f:")) != EOF) {
    switch (c)  {
    case 'v':
      version();
      exit(0);
      break;
    case 'b':
      blackness = atoi(optarg);
      if (blackness <= 0) err ++;
      break;
    case 'w':
      numerator_x = atoi(optarg);
      denominator_x = strchr(optarg,'/')
	? atoi(strchr(optarg,'/')+1) : 1;
      if (numerator_x <= 0 || denominator_x <= 0) err ++;
      break;
    case 'h':
      numerator_y = atoi(optarg);
      denominator_y = strchr(optarg,'/')
	? atoi(strchr(optarg,'/')+1) : 1;
      if (numerator_y <= 0 || denominator_y <= 0) err ++;
      break;
    case 'f':
      numerator_x = numerator_y = atoi(optarg);
      denominator_x = denominator_y = strchr(optarg,'/')
	? atoi(strchr(optarg,'/')+1) : 1;
      if (numerator_x <= 0 || denominator_x <= 0) err ++;
      break;
    case '?':
      err ++;
      break;
    }
  }
  if (err) {
    version();
    usage();
    exit(1);
  }
  if (optind < argc && freopen(argv[optind], "r", stdin) == NULL) {
    perror(argv[optind]);
    exit(1);
  }
  charcount = processHeader();
  for (; charcount > 0; charcount --) {
    processChar();
  }
  processFooter();
  exit(0);
  /* NOTREACHED */
}

static int
processHeader(void)
{
  startfont();
  font();
  size();
  fontboundingbox();
  processProperties();
  return chars();
}

static void
processFooter(void)
{
  endfont();
}

static void
processProperties(void)
{
  int	arg;

  get_line();
  if (beginwith(linebuf, "STARTPROPERTIES")) { /* optional */
    if (sscanf(linebuf, "STARTPROPERTIES %d", &arg) != 1
	|| arg < 0) {
      error("STARTPROPERTIES has illegal or no arguments\n");
    }
    put_line();
    for (; arg > 0; arg --) {
      get_line();
      if (beginwith(linebuf, "FONT_ASCENT")) {
	hresize("FONT_ASCENT");
      } else if (beginwith(linebuf, "FONT_DESCENT")) {
	yresize("FONT_DESCENT");
      } else if (beginwith(linebuf, "PIXEL_SIZE")) {
	hresize("PIXEL_SIZE");
      } else if (beginwith(linebuf, "POINT_SIZE")) {
	hresize("POINT_SIZE");
      } else if (beginwith(linebuf, "AVERAGE_WIDTH")) {
	wresize("AVERAGE_WIDTH");
      }
      put_line();
    }
    get_line();
    if (!beginwith(linebuf, "ENDPROPERTIES")) {
      error("ENDPROPERTIES expected\n");
    }
    put_line();
    get_line();
  }
}

static void
wresize(char *prop)
{
  int	arg;
  char	form[100];
  char	errbuf[100];
  sprintf(form, "%s %%d", prop);
  if (sscanf(linebuf, form, &arg) != 1) {
    sprintf(errbuf, "%s has illegal or no arguments\n", prop);
    error(errbuf);
  }
  sprintf(linebuf, "%s %d", prop, WRESIZE(arg));
}

static void
hresize(char *prop)
{
  int	arg;
  char	form[100];
  char	errbuf[100];
  sprintf(form, "%s %%d", prop);
  if (sscanf(linebuf, form, &arg) != 1) {
    sprintf(errbuf, "%s has illegal or no arguments\n", prop);
    error(errbuf);
  }
  sprintf(linebuf, "%s %d", prop, HRESIZE(arg));
}

static void
yresize(char *prop)
{
  int	arg;
  char	form[100];
  char	errbuf[100];
  sprintf(form, "%s %%d", prop);
  if (sscanf(linebuf, form, &arg) != 1) {
    sprintf(errbuf, "%s has illegal or no arguments\n", prop);
    error(errbuf);
  }
  sprintf(linebuf, "%s %d", prop, YRESIZE(arg));
}

static void
startfont(void)
{
  char	arg[BUFSIZE];

  get_line();
  if (!beginwith(linebuf, "STARTFONT")) {
    error("STARTFONT expected\n");
  }
  if (sscanf(linebuf, "STARTFONT %s", arg) != 1) {
    error("STARTFONT has illegal or no arguments\n");
  }
  if (strcmp(arg, "2.1")) {
    error("STARTFONT version error\n");
  }
  put_line();
  sprintf(linebuf,
	  "COMMENT This font was automatically resized by bdfresize-%s",
	  VERSION);
  put_line();
  sprintf(linebuf,
	  "COMMENT   mag_x = %d/%d, mag_y = %d/%d, blackness = %d",
	  numerator_x, denominator_x, numerator_y, denominator_y, blackness);
  put_line();
}

static void
font(void)
{
  char *cp;
  get_line();
  if (!beginwith(linebuf, "FONT")) {
    error("FONT expected\n");
  }
  cp = linebuf + 4;
  while (*cp && isspace((int)*cp)) cp++;
  if (!*cp) {
    error("FONT has no arguments\n");
  }
  if (*cp == '+' || *cp == '-') { /* xlfd */
    resize_xlfd(cp);
  }
  put_line();
}

static void
resize_xlfd(char *xlfd)
{
  char	buf[BUFSIZE];
  char	*p, *q;

  buf[0] = '\0';

  if (!(q = strchr(p = xlfd, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* XFontNameRegistry */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* FOUNDRY */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* FAMILY_NAME */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* WEIGHT_NAME */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* SLANT */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* SETWIDTH_NAME */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* ADD_STYLE_NAME */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  if (!isnum(p)) { fputs(p, stdout); putchar('-'); goto parseerror; }
  sprintf(buf + strlen(buf), "%d", HRESIZE(atoi(p))); strcat(buf, "-");
  /* PIXEL_SIZE */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  if (!isnum(p)) { fputs(p, stdout); putchar('-'); goto parseerror; }
  sprintf(buf + strlen(buf), "%d", HRESIZE(atoi(p))); strcat(buf, "-");
  /* POINT_SIZE */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* RESOLUTION_X */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* RESOLUTION_Y */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* SPACING */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  if (!isnum(p)) { fputs(p, stdout); putchar('-'); goto parseerror; }
  sprintf(buf + strlen(buf), "%d", WRESIZE(atoi(p))); strcat(buf, "-");
  /* AVERAGE_WIDTH */

  if (!(q = strchr(p = q + 1, '-'))) goto parseerror;
  *q = '\0';
  strcat(buf, p); strcat(buf, "-");
  /* CHARSET_REGISTRY */

  if ((q = strchr(p = q + 1, '-'))) goto parseerror;
  strcat(buf, p);
  /* CHARSET_ENCODING */

  strcpy(xlfd, buf);
  return;

 parseerror:
  fprintf(stderr, "Cannot parse XLFD '%s'\n", xlfd);
  strcat(buf, p);			/* rest */

  strcpy(xlfd, buf);
  return;
}

static int
isnum(char *p)
{
  if (!*p) return 0;
  while (*p && isdigit((int)*p)) {
    p ++;
  }
  if (*p) return 0;

  return 1;
}

static void
size(void)
{
  int	arg1, arg2, arg3;
  int	newsize;

  get_line();
  if (!beginwith(linebuf, "SIZE")) {
    error("SIZE expected\n");
  }
  if (sscanf(linebuf, "SIZE %d %d %d", &arg1, &arg2, &arg3) != 3
      || arg1 <= 0 || arg2 <= 0 || arg3 <= 0) {
    error("SIZE has illegal arguments\n");
  }
  newsize = HRESIZE(arg1);
  sprintf(linebuf, "SIZE %d %d %d", newsize, arg2, arg3);
  put_line();
}

static void
fontboundingbox(void)
{
  int	arg1, arg2, arg3, arg4;
  int	new1, new2, new3, new4;

  get_line();
  if (!beginwith(linebuf, "FONTBOUNDINGBOX")) {
    error("FONTBOUNDINGBOX expected\n");
  }
  if (sscanf(linebuf, "FONTBOUNDINGBOX %d %d %d %d",
	     &arg1, &arg2, &arg3, &arg4) != 4
      || arg1 <= 0 || arg2 <= 0) {
    error("FONTBOUNDINGBOX has illegal arguments\n");
  }
  new1 = WRESIZE(arg1);
  new2 = HRESIZE(arg2);
  new3 = XRESIZE(arg3);
  new4 = YRESIZE(arg4);
  sprintf(linebuf, "FONTBOUNDINGBOX %d %d %d %d", new1,new2,new3,new4);
  put_line();
}

static int
chars(void)
{
  int	arg;

  if (!beginwith(linebuf, "CHARS")) {
    error("CHARS expected\n");
  }
  if (sscanf(linebuf, "CHARS %d", &arg) != 1 || arg < 0) {
    error("CHARS has illegal or no arguments\n");
  }
  put_line();
  return arg;
}

static void
endfont(void)
{
  get_line();
  if (!beginwith(linebuf, "ENDFONT")) {
    error("ENDFONT expected\n");
  }
  put_line();
}

void
get_line(void)
{
  while (fgets(linebuf, sizeof(linebuf), stdin)) {
    line ++;
    linebuf[strlen(linebuf)-1] = '\0';
    if (linebuf[0] == '\0' || beginwith(linebuf, "COMMENT")) {
      put_line();
    } else {
      return;
    }
  }
  error("EOF detected\n");
}

void
put_line(void)
{
  puts(linebuf);
}

int
beginwith(char *l, char *s)
{
  return !strncmp(l, s, strlen(s));
}

void error(char *fmt)
{
  char	buf[100];
  strcat(buf, "bdfresize: line %d: ");
  strcat(buf, fmt);
  fprintf(stderr, buf, line);
  exit(1);
}

static void
usage(void)
{
  fprintf(stderr, "usage: bdfresize [-options] [bdffile]\n");
  fprintf(stderr, "  -v : show version\n");
  fprintf(stderr, "  -b <number> : blackness (default = 4)\n");
  fprintf(stderr, "  -f <factor> : \n");
  fprintf(stderr, "  -w <factor> : (width only)\n");
  fprintf(stderr, "  -h <factor> : (height only)\n");
  fprintf(stderr, "    (<factor> ::= digits | digits/digits)\n");
}

static void
version(void)
{
  fprintf(stderr, "bdfresize: version %s\n", VERSION);
}
