noinst_LIBRARIES = libirc.a

libirc_a_SOURCES = ServerConnection.cpp ServerConnection.h \
                       Commands.cpp Commands.h \
                       ConfigHandler.cpp ConfigHandler.h ConfigValue.h \
                       LostIRCApp.cpp LostIRCApp.h \
                       Parser.cpp Parser.h \
                       Socket.cpp Socket.h \
                       Utils.cpp Utils.h \
                       Channel.cpp Channel.h \
                       DCC.cpp DCC.h \
                       FrontEnd.cpp FrontEnd.h \
                       irc_defines.h \
                       LostIRC.h

libirc_a_LIBS = $(GLIBMMDEPS_LIBS)
INCLUDES = $(GLIBMMDEPS_CFLAGS) -I$(top_srcdir)/intl
