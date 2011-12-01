#
# (C) Copyright 2004
# Texas Instruments, <www.ti.com>
#
# SDP2430 boad uses OMAP2430 (ARM1136) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# SDP2430 has 1 bank of 32MB or 64MB mDDR-SDRAM on CS0
# SDP2430 has 1 bank of 32MB or 00MB mDDR-SDRAM on CS1
# Physical Address:
# 8000'0000 (bank0)
# A000/0000 (bank1) ES2 will be configurable
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)

# For use with external or internal boots.
TEXT_BASE = 0x80e80000


# Handy to get symbols to debug ROM version.
#TEXT_BASE = 0x0
#TEXT_BASE = 0x08000000
#TEXT_BASE = 0x04000000
