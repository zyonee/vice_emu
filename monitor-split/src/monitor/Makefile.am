INCLUDES = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/arch/unix/readline \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/vdrive

noinst_LIBRARIES = libmonitor.a

libmonitor_a_SOURCES = \
	asm6502.c \
	asm6502dtv.c \
	asm6809.c \
	asmz80.c \
	asm.h \
	mon_assemble6502.c \
	mon_assemble6809.c \
	mon_assemblez80.c \
	mon_assemble.h \
	mon_asm6502_lex.c \
	mon_asm6502_parse.c \
	mon_asm6502_parse.h \
	mon_breakpoint.c \
	mon_breakpoint.h \
	mon_command.c \
	mon_command.h \
	mon_disassemble.c \
	mon_disassemble.h \
	mon_drive.c \
	mon_drive.h \
	mon_file.c \
	mon_file.h \
	mon_lex.c \
	mon_memory.c \
	mon_memory.h \
	mon_parse.c \
	mon_parse.h \
	mon_register6502.c \
	mon_register6502dtv.c \
	mon_register6809.c \
	mon_registerz80.c \
	mon_register.h \
	mon_ui.c \
	mon_ui.h \
	mon_util.c \
	mon_util.h \
	monitor.c \
	monitor_network.c \
	monitor_network.h \
	montypes.h
