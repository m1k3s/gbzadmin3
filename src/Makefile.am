## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = gbzadmin3

gbzadmin3_SOURCES = \
	main.cxx \
	utilities.cxx utilities.h \
	gSocket.cxx gSocket.h \
	gListServer.cxx gListServer.h \
	gbzadmin.cxx gbzadmin.h \
	view.cxx view.h \
	msg_view.cxx msg_view.h \
	player_view.cxx player_view.h \
	cmdLine.cxx cmdLine.h \
	server_vars_view.cxx server_vars_view.h \
	game_stat_view.cxx game_stat_view.h \
	players.cxx players.h \
	parser.cxx parser.h \
	serverlist_view.cxx serverlist_view.h \
	configfile.cxx configfile.h

gbzadmin3_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -lcurl
