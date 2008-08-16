# Makefile.am for the SDL version of `libarch.a'.

INCLUDES = \
	@ARCH_INCLUDES@ \
	@FFMPEG_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	archdep.c \
	archdep.h \
	blockdev.c \
	console.c \
	fullscreenarch.h \
	joy.c \
	joy.h \
	kbd.c \
	kbd.h \
	mousedrv.c \
	mousedrv.h \
	sdlmain.c \
	signals.c \
	rs232.c \
	types.h \
	ui.c \
	ui.h \
	uic64.c \
	uicmdline.c \
	uimenu.h \
	uimon.c \
	video.c \
	videoarch.h \
	vsidui.c \
	vsyncarch.c
