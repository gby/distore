
all-local: iniparser-build
clean-local: iniparser-clean

iniparser-build: 
	(cd $(srcdir)/iniparser3.0b && $(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(AM_MAKEFLAGS) )

iniparser-clean: 
	(cd $(srcdir)/iniparser3.0b && $(MAKE) $(AM_MAKEFLAGS) clean)
	rm -rf $(srcdir)/iniparser3.0b/libiniparser.* clean

EXTRA_DIST =  iniparser3.0b
