AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/drive/iec/c64exp \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/tape \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/viciisc \
	-I$(top_srcdir)/src/viciivsid \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/tapeport

noinst_LIBRARIES = libc65.a

libc65_a_SOURCES = \
	c65-cmdline-options.c \
	c65-cmdline-options.h \
	c65-resources.c \
	c65-resources.h \
	c65-snapshot.c \
	c65-snapshot.h \
	c65.c \
	c65.h \
	c65bus.c \
	c65cia.h \
	c65cia1.c \
	c65cia2.c \
	c65cpu.c \
	c65drive.c \
	c65export.c \
	c65export.h \
	c65fastiec.c \
	c65fastiec.h \
	c65gluelogic.c \
	c65gluelogic.h \
	c65iec.c \
	c65iec.h \
	c65io.c \
	c65keyboard.c \
	c65keyboard.h \
	c65mem.c \
	c65mem.h \
	c65meminit.c \
	c65meminit.h \
	c65memlimit.c \
	c65memlimit.h \
	c65memrom.c \
	c65memrom.h \
	c65memsnapshot.c \
	c65memsnapshot.h \
	c65model.c \
	c65model.h \
	c65parallel.c \
	c65parallel.h \
	c65pla.c \
	c65pla.h \
	c65printer.c \
	c65rom.c \
	c65rom.h \
	c65romset.c \
	c65rsuser.c \
	c65rsuser.h \
	c65sound.c \
	c65video.c
