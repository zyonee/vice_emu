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
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	archdep.c \
	archdep.h \
	blockdev.c \
	console.c \
	fullscreen.c \
	fullscreenarch.h \
	joy.c \
	joy.h \
	kbd.c \
	kbd.h \
	menu_common.c \
	menu_common.h \
	menu_reset.c \
	menu_reset.h \
	menu_speed.c \
	menu_speed.h \
	mousedrv.c \
	mousedrv.h \
	sdlmain.c \
	signals.c \
	types.h \
	ui.c \
	ui.h \
	uic128.c \
	uic64.c \
	uic64dtv.c \
	uicbm2.c \
	uicmdline.c \
	uifilereq.c \
	uifilereq.h \
	uimenu.c \
	uimenu.h \
	uimon.c \
	uipet.c \
	uiplus4.c \
	uivic20.c \
	video.c \
	videoarch.h \
	vsidui.c \
	vsyncarch.c
