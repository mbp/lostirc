SUBDIRS = src
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = lostirc.desktop \
             lostirc.1

dist-hook: lostirc.spec
	cp lostirc.spec $(distdir)

Applicationsdir = $(datadir)/gnome/apps/Internet
Applications_DATA = lostirc.desktop
man_MANS = lostirc.1
