# contrib/pg_backtrace/Makefile

MODULE_big = pg_backtrace
OBJS = pg_backtrace.o
PGFILEDESC = "PG_BACKTRACE - tool for dumping stacks for errors"

PG_CPPFLAGS = -I$(libpq_srcdir)
SHLIB_LINK = $(libpq)

EXTENSION = pg_backtrace
DATA = pg_backtrace--1.0.sql

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_backtrace
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
