# Makefile.am for `libjoystickdrv.a'.

INCLUDES = \
	@ARCH_INCLUDES@ \
        -I$(top_builddir)/src \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libjoystickdrv.a

# These sources are linked only under certain circumstances.
EXTRA_libjoystickdrv_a_SOURCES = \
	joy_directinput.c \
	joy_winmm.c \
	joy_unix.c \
	joy_unix_old.c \
	joy_unix_digi_old.c \
	joy_allegro.c

# These sources are always built.
libjoystickdrv_a_SOURCES = 

libjoystickdrv_a_DEPENDENCIES = \
	@JOYSTICK_DRIVERS@

libjoystickdrv_a_LIBADD = \
	@JOYSTICK_DRIVERS@

EXTRA_DIST =
