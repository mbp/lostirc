noinst_LIBRARIES = libirc.a

libirc_a_SOURCES = ServerConnection.cpp \
                       Commands.cpp \
                       ConfigHandler.cpp \
                       LostIRCApp.cpp \
                       Parser.cpp \
                       Socket.cpp \
                       Utils.cpp \
                       Events.cpp

libirc_a_LIBS = @SIGC_LIBS@ @GLIB_LIBS@
INCLUDES = @SIGC_CFLAGS@ @GLIB_CFLAGS@
