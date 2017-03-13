PREFIX=/usr
BIN_DIR=$(PREFIX)/bin
ZSH_FPATH=$(PREFIX)/share/zsh/site-functions
INCLUDE_PATH=$(PREFIX)/include
PKGINCLUDE_PATH=$(INCLUDE_PATH)/knapsack

PKGCONFIG = pkg-config
CPPFLAGS_EXTRA = -I. -DNDEBUG
#CXXFLAGS_EXTRA = -std=c++11

ifdef WARN
LDFLAGS_EXTRA = \
	-Wl,-z,defs \
	-Wl,--no-undefined \
	-Wl,--no-allow-shlib-undefined \
	-Wl,--no-undefined-version
CXXFLAGS_EXTRA ::= $(CXXFLAGS_EXTRA) \
	-Wpedantic \
	-pedantic \
	-Wall \
	-Wextra \
	-Wformat=2 \
	-Wunused-variable \
	-Wunused-parameter \
	-Winit-self \
	-Wmissing-include-dirs \
	-Wswitch-default \
	-Wunused \
	-Wunused-parameter \
	-Wstrict-aliasing=1 \
	-Wstrict-overflow=5 \
	-Wfloat-equal \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wcast-align \
	-Wwrite-strings \
	-Wsign-compare \
	-Wmissing-field-initializers \
	-Wnormalized=nfkc \
	-Wpacked \
	-Wredundant-decls \
	-Winvalid-pch \
	-Wvolatile-register-var \
	-Wdisabled-optimization \
	-Wlogical-op \
	-Wvla \
	-Wabi \
	-Wstrict-null-sentinel \
	-Woverloaded-virtual \
	-Wsign-promo \
	-Wnoexcept \
	-Wmissing-declarations \
	-Wmissing-format-attribute \
	-Wunused-but-set-variable \
	-Wunused-but-set-parameter \
	-Wdelete-non-virtual-dtor \
	-Wint-to-pointer-cast \
	-Wcomment \
	-Wswitch-bool \
	-Wsizeof-array-argument \
	-Wbool-compare \
	-Wshift-negative-value \
	-Wshift-overflow \
	-Wshift-overflow=2 \
	-Wnull-dereference \
	-Wduplicated-cond \
	-Wsign-conversion \
	-Wold-style-cast \
	-Wmissing-noreturn \
	-Wunreachable-code
endif

.PHONY: FORCE all install uninstall clean distclean maintainer-clean check

all: bin/knapsack

bin/knapsack: knapsack/knapsack.cc knapsack/knapsack.h
	test -d bin || mkdir bin
	$(CXX) $(CPPFLAGS_EXTRA) $(CXXFLAGS_EXTRA) $(LDFLAGS_EXTRA) \
		$(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) \
		-o bin/knapsack knapsack/knapsack.cc

install: knapsack
	install -d $(DESTDIR)$(BIN_DIR)
	install -d $(DESTDIR)/$(ZSH_FPATH)
	install -d $(DESTDIR)/$(INCLUDE_PATH)
	install bin/knapsack $(DESTDIR)$(BIN_DIR)/knapsack
	install -m 644 zsh/_knapsack $(DESTDIR)/$(ZSH_FPATH)/_knapsack
	install -m 644 knapsack/knapsack.h $(DESTDIR)/$(INCLUDE_PATH)/knapsack.h

uninstall: FORCE
	rm -f $(DESTDIR)/$(BIN_DIR)/knapsack
	rm -f $(DESTDIR)/$(ZSH_FPATH)/_knapsack
	rm -f $(DESTDIR)/$(INCLUDE_PATH)/_knapsack.h
	rmdir -p $(DESTDIR)/$(BIN_DIR)
	rmdir -p $(DESTDIR)/$(ZSH_FPATH)
	rmdir -p $(DESTDIR)/$(INCLUDEPATH)

clean: FORCE
	rm -f bin/knapsack *.o *.obj knapsack/*.o knapsack/*.obj
	! test -d bin || rmdir bin

distclean: clean FORCE
	rm -f ./knapsack-*.asc ./knapsack-*.tar.* ./knapsack-*.tar ./knapsack-*.zip

maintainer-clean: distclean FORCE

check: bin/knapsack FORCE
	contrib/testsuite bin/knapsack

FORCE:
