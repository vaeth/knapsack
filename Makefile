PREFIX=/usr
BIN_DIR=$(PREFIX)/bin
ZSH_FPATH=$(PREFIX)/share/zsh/site-functions
INCLUDE_PATH=$(PREFIX)/include/knapsack

MAKE_CPPFLAGS=-I. -DNDEBUG

.PHONY: FORCE all install uninstall clean distclean maintainer-clean

all: knapsack

knapsack: src/knapsack.cc src/knapsack.h src/knapsack_output.h
	$(CXX) $(MAKE_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) \
		-o knapsack src/knapsack.cc

install: knapsack
	install -d $(DESTDIR)$(BIN_DIR)
	install -d $(DESTDIR)/$(ZSH_FPATH)
	install -d $(DESTDIR)/$(INCLUDE_PATH)
	install knapsack $(DESTDIR)$(BIN_DIR)/knapsack
	install -m 644 zsh/_knapsack $(DESTDIR)/$(ZSH_FPATH)/_knapsack
	install -m 644 src/knapsack.h $(DESTDIR)/$(INCLUDE_PATH)/knapsack.h
	install -m 644 src/knapsack_output.h \
		$(DESTDIR)/$(INCLUDE_PATH)/knapsack_output.h

uninstall: FORCE
	rm -f $(DESTDIR)/$(BIN_DIR)/knapsack
	rm -f $(DESTDIR)/$(ZSH_FPATH)/_knapsack
	rm -f $(DESTDIR)/$(INCLUDE_PATH)/_knapsack.h
	rm -f $(DESTDIR)/$(INCLUDE_PATH)/_knapsack_output.h
	rmdir -p $(DESTDIR)/$(BIN_DIR)
	rmdir -p $(DESTDIR)/$(ZSH_FPATH)
	rmdir -p $(DESTDIR)/$(INCLUDEPATH)

clean: FORCE
	rm -f src/knapsack ./knapsack *.o *.obj src/*.o src/*.obj

distclean: clean FORCE
	rm -f ./knapsack-*.asc ./knapsack-*.tar.* ./knapsack-*.tar ./knapsack-*.zip

maintainer-clean: distclean FORCE

test: knapsack
	contrib/testsuite ./knapsack

FORCE:
