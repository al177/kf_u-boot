#
# (C) Copyright 2009
# Texas Instruments, <www.ti.com>
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= lib$(CPU).a

START	= start.o
OBJS	= interrupts.o cpu.o mmc.o clock.o mem.o spi.o sys_info.o omap4.o syslib.o sparse.o
SOBJS	= lowlevel_init.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c) $(SOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c)\
			$(SOBJS:.o=.S)> $@

sinclude .depend

#########################################################################
