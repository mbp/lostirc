bin_PROGRAMS = lostirc

lostirc_SOURCES =  main.cpp \
                   MainWindow.h MainWindow.cpp \
                   MainNotebook.h MainNotebook.cpp \
                   Tab.h Tab.cpp \
                   Prefs.h Prefs.cpp \
                   GuiCommands.h GuiCommands.cpp \
                   Entry.cpp Entry.h \
                   StatusBar.cpp StatusBar.h \
                   TextWidget.cpp TextWidget.h \
                   NickList.cpp NickList.h \
                   DCCList.cpp DCCList.h

lostirc_LDADD = ../libirc/libirc.a

INCLUDES = $(DEPS_CFLAGS) \
           -I$(top_srcdir)/src/libirc \
           -I$(top_srcdir)/intl \
           -DLOCALEDIR=\"$(datadir)/locale\" \
           -DLOSTIRC_DATADIR=\"$(datadir)\"

LIBS = $(DEPS_LIBS)
