## Process this file with automake to produce Makefile.in
##
## save this - 	    ln -s ../$$gladefile $(DESTDIR)$(pkgdatadir)/$$gladefile; \
## save this too - 	    rm -f $(DESTDIR)$(pkgdatadir)/$$gladefile; \

SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh \
	gbzadmin3.glade

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi
	for gladefile in gbzadmin3.glade; do \
	  if test -f $$gladefile; then \
            $(INSTALL_DATA) $$gladefile $(DESTDIR)$(pkgdatadir); \
	  fi \
	done

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
