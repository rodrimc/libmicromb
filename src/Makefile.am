#bin_PROGRAMS = mb_test

lib_LTLIBRARIES=libmicromb.la

libmicromb_la_SOURCES = mb/util.c \
												mb/mb.c

libmicromb_la_CFLAGS = $(GSTREAMER_VIDEO_CFLAGS) -Wall

libmicromb_la_LDFLAGS = -shared -fPIC -rpath,$(LIBDIR) \
												$(GSTREAMER_VIDEO_LIBS)

libmicromb_la_LDFLAGS += -version-info 1:1:1

micrombincludedir = $(includedir)/micromb-$(VERSION)
micrombinclude_HEADERS = mb/mb.h \
												 mb/events.h

