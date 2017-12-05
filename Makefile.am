AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = mbs
mbs_SOURCES = main.c \
		mbs.h \
		mbs.c \
		window.h \
		window.c \
		argtable3/argtable3.h \
		argtable3/argtable3.c
AM_LDFLAGS = -lncursesw
mbs_LDADD = $(NCURSESW_LIBS)
