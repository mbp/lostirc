SUBDIRS = src
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = lostirc.desktop

dist-hook: lostirc.spec
	cp lostirc.spec $(distdir)

Applicationsdir = $(datadir)/gnome/apps/Internet
Applications_DATA = lostirc.desktop

