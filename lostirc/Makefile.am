SUBDIRS = src
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = lostirc.desktop \
             lostirc.1

dist-hook: lostirc.spec
	cp lostirc.spec $(distdir)

if GNOME
Gnomedir = $(datadir)/gnome/apps/Internet
Gnome_DATA = lostirc.desktop
endif

if KDE
Kdedir = $(prefix)/share/applnk
Kde_DATA = lostirc.desktop
endif

man_MANS = lostirc.1
