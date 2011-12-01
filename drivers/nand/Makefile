include $(TOPDIR)/config.mk

LIB := libnand.a

OBJS := nand.o nand_base.o nand_ids.o nand_ecc_256.o nand_ecc_512.o nand_bbt.o nand_util.o
all:	$(LIB)

$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend
