bin_PROGRAMS = lostirc

lostirc_SOURCES =  main.cpp \
                   MainWindow.h MainWindow.cpp \
                   MainNotebook.h MainNotebook.cpp \
                   Tab.h Tab.cpp \
                   Prefs.h Prefs.cpp \
                   GuiCommands.h GuiCommands.cpp \
                   Entry.cpp Entry.h \
                   StatusBar.cpp StatusBar.h

lostirc_LDADD = ../libirc/libirc.a

INCLUDES = $(DEPS_CFLAGS) \
           -I$(top_srcdir)/src/libirc

LIBS = $(DEPS_LIBS)
