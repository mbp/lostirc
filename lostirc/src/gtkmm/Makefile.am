bin_PROGRAMS = lostirc

lostirc_SOURCES =  main.cpp \
                   MainWindow.h MainWindow.cpp \
                   MainNotebook.h MainNotebook.cpp \
                   Tab.h Tab.cpp \
                   GuiCommands.h GuiCommands.cpp \
                   Entry.cpp Entry.h

lostirc_LDADD = ../libirc/libirc.a

INCLUDES = @GTKMM_CFLAGS@ \
           -I$(top_srcdir)/src/libirc
