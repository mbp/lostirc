SUBDIRS = intl po src

EXTRA_DIST = config.rpath  lostirc.desktop \
             lostirc.1 lostirc.png

dist-hook: lostirc.spec
	cp lostirc.spec $(distdir)

if GNOME
Gnomedir = $(datadir)/applications
Gnome_DATA = lostirc.desktop
endif

if KDE
Kdedir = $(prefix)/share/applnk
Kde_DATA = lostirc.desktop
endif

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = lostirc.png

man_MANS = lostirc.1

ACLOCAL_AMFLAGS = -I m4
