SUBDIRS = \
	c64exp \
	plus4exp

INCLUDES = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive

noinst_LIBRARIES = libdriveiec.a

libdriveiec_a_SOURCES = \
	cia1571d.c \
	cia1581d.c \
	ciad.h \
	iec-cmdline-options.c \
	iec-cmdline-options.h \
	iec-resources.c \
	iec-resources.h \
	iec.c \
	iecrom.c \
	iecrom.h \
	memiec.c \
	memiec.h \
	via1d1541.c \
	via1d1541.h \
	wd1770.c \
	wd1770.h \
	via4000.c \
	via4000.h \
	pc8477.c \
	pc8477.h

.PHONY: libdriveiecc64exp libdriveiecplus4exp

libdriveiecc64exp:
	(cd c64exp; $(MAKE))
libdriveiecplus4exp:
	(cd plus4exp; $(MAKE))
