noinst_LIBRARIES = libirc.a

libirc_a_SOURCES = ServerConnection.cpp ServerConnection.h \
                       Commands.cpp Commands.h \
                       ConfigHandler.cpp ConfigHandler.h \
                       LostIRCApp.cpp LostIRCApp.h \
                       Parser.cpp Parser.h \
                       Socket.cpp Socket.h \
                       Utils.cpp Utils.h \
                       Events.cpp Events.h \
                       Channel.cpp Channel.h \
                       DCC.cpp DCC.h \
                       FrontEnd.cpp FrontEnd.h \
                       irc_defines.h

libirc_a_LIBS = @SIGC_LIBS@ @GLIB_LIBS@
INCLUDES = @SIGC_CFLAGS@ @GLIB_CFLAGS@
