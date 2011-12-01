#
# (C) Copyright 2006
# Texas Instruments, <www.ti.com>
#
# SDP3430 board uses OMAP3430 (ARM-CortexA8) cpu
# see http://www.ti.com/ for more information on Texas Instruments
#
# SDP3430 has 1 bank of 32MB or 64MB mDDR-SDRAM on CS0
# SDP3430 has 1 bank of 32MB or 00MB mDDR-SDRAM on CS1
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
