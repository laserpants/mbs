AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = mbs testexe
AM_LDFLAGS = -lncursesw
mbs_SOURCES = main.c \
		mbs.h \
		mbs.c \
		window.h \
		window.c \
		argtable3/argtable3.h \
		argtable3/argtable3.c
mbs_LDADD = $(NCURSESW_LIBS)
testexe_SOURCES = tests/main.c \
		mbs.h \
		mbs.c \
		window.h \
		window.c \
		argtable3/argtable3.h \
		argtable3/argtable3.c
testexe_LDADD = $(NCURSESW_LIBS)
TESTS = testexe
