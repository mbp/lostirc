bin_PROGRAMS = lostirc

lostirc_SOURCES =  main.cpp \
                   MainWindow.h MainWindow.cpp \
                   MainNotebook.h MainNotebook.cpp \
                   Tab.h Tab.cpp \
                   GuiCommands.h GuiCommands.cpp

lostirc_LDADD = ../libirc/libirc.a

INCLUDES = @GTKMM_CFLAGS@ \
           -I../libirc
