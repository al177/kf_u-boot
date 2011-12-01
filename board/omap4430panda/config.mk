#
# (C) Copyright 2006-2009
# Texas Instruments, <www.ti.com>
#
# SDP4430 board uses ARM-CortexA9 cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# Physical Address:
# 8000'0000 (bank0)
# A000/0000 (bank1)
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)

# For use with external or internal boots.
TEXT_BASE = 0x80e80000


# Handy to get symbols to debug ROM version.
#TEXT_BASE = 0x0
#TEXT_BASE = 0x08000000
#TEXT_BASE = 0x04000000
