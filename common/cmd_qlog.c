/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * Code used from linux/kernel/printk.c
 * Copyright (C) 1991, 1992  Linus Torvalds
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Comments:
 *
 * After relocating the code, the environment variable "loglevel" is
 * copied to console_loglevel.  The functionality is similar to the
 * handling in the Linux kernel, i.e. messages logged with a priority
 * less than console_loglevel are also output to stdout.
 *
 * If you want messages with the default level (e.g. POST messages) to
 * appear on stdout also, make sure the environment variable
 * "loglevel" is set at boot time to a number higher than
 * default_message_loglevel below.
 */

/*
 * Logbuffer handling routines
 */

#include <common.h>
#include <command.h>
#include <devices.h>
#if defined(CONFIG_FREE_LOG_BASE)
/* Local prototypes */
static void qlogbuff_putc (const char c);
static void qlogbuff_puts (const char *s);
static int qlogbuff_printk(const char *line);

static char buf[1024];

#define LOG_LENGTH   0x1000
#define LOGBUFF_MASK	(LOG_LENGTH-1)
static unsigned char *log_buf = NULL;
static unsigned long *ext_log_size;
static unsigned long *ext_log_start;
static unsigned long *ext_logged_chars;
#define log_size (*ext_log_size)
#define log_start (*ext_log_start)
#define logged_chars (*ext_logged_chars)

void qlog_init(void)
{
	log_buf = (unsigned char *)(CONFIG_FREE_LOG_BASE-LOG_LENGTH);
	/* initialization */
	memset (log_buf, 0, sizeof (LOG_LENGTH));

 	ext_log_start = (unsigned long *)(CONFIG_FREE_LOG_BASE)-3;
	ext_log_size = (unsigned long *)(CONFIG_FREE_LOG_BASE)-2;
	ext_logged_chars = (unsigned long *)(CONFIG_FREE_LOG_BASE)-1;
	log_start    = 0;
	log_size     = 0;
	logged_chars = 0;
}

static void qlogbuff_putc (const char c)
{
	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	qlogbuff_printk (buf);
}

static void qlogbuff_puts (const char *s)
{
	qlogbuff_printk (s);
}

void qlogbuff_log(char *msg)
{
	qlogbuff_printk (msg);
}

int qlogbuff_printk(const char *line)
{
	int i;
	char *msg, *p, *buf_end;
	strcpy (buf , line);
	i = strlen (line);
	buf_end = buf + i;
	for (p = buf ; p < buf_end; p++) {
		msg = p;
		for (; p < buf_end; p++) {
			log_buf[(log_start+log_size) & LOGBUFF_MASK] = *p;
			if (log_size < (LOG_LENGTH-0x10))
				log_size++;
			else
				log_start++;

			logged_chars++;
			if (*p == '\n') {
				break;
			}
		}
	}
	return i;
}

/*
 * Subroutine:  do_log
 *
 * Description: Handler for 'log' command..
 *
 * Inputs:	argv[1] contains the subcommand
 *
 * Return:      None
 *
 */
static int do_qlog (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *s;
	unsigned long i;

	if (strcmp(argv[1],"append") == 0) {
		/* Log concatenation of all arguments separated by spaces */
		for (i=2; i<argc; i++) {
			qlogbuff_printk (argv[i]);
			qlogbuff_putc ((i<argc-1) ? ' ' : '\n');
		}
		return 0;
	}

	switch (argc) {

	case 2:
		if (strcmp(argv[1],"show") == 0) {
			for (i=0; i < (log_size&LOGBUFF_MASK); i++) {
				s = (char *)log_buf+((log_start+i)&LOGBUFF_MASK);
				putc (*s);
			}
			return 0;
		} else if (strcmp(argv[1],"reset") == 0) {
			log_start    = 0;
			log_size     = 0;
			logged_chars = 0;
			return 0;
		} else if (strcmp(argv[1],"info") == 0) {
			printf ("Logbuffer   at  %08lx\n", (unsigned long)log_buf);
			printf ("log_start    =  %08lx\n", log_start);
			printf ("addr = %x   log_size     =  %08lx\n",ext_log_size, log_size);
			printf ("logged_chars =  %08lx\n", logged_chars);
			return 0;
		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;

	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

U_BOOT_CMD(
	qlog,     255,	0,	do_qlog,
	"qlog     - manipulate logbuffer\n",
	"info   - show pointer details\n"
	"qlog reset  - clear contents\n"
	"qlog show   - show contents\n"
	"qlog append <msg> - append <msg> to the logbuffer\n"
);
#endif /* (CONFIG_LOGBUFFER) */
