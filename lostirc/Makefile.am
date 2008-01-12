SUBDIRS = po src

EXTRA_DIST = config.rpath  lostirc.desktop \
             lostirc.1 lostirc.png m4

dist-hook: lostirc.spec
	cp lostirc.spec $(distdir)

if DESKTOPFILE
Desktopdir = $(datadir)/applications
Desktop_DATA = lostirc.desktop
endif

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = lostirc.png

man_MANS = lostirc.1

ACLOCAL_AMFLAGS = -I m4
