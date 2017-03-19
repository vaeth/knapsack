PREFIX=/usr
BIN_DIR=$(PREFIX)/bin
ZSH_FPATH=$(PREFIX)/share/zsh/site-functions
INCLUDE_PATH=$(PREFIX)/include
PKGINCLUDE_PATH=$(INCLUDE_PATH)/knapsack

PKGCONFIG = pkg-config
BOOST_CFLAGS = -I$(INCLUDE_PATH)
BOOST_LIBS = -lboost_program_options-mt

CPPFLAGS_EXTRA = -I.
CXXFLAGS_EXTRA = $(BOOST_CFLAGS)
LDFLAGS_EXTRA = $(BOOST_LIBS)

CPPFLAGS_OPTIMIZE = \
	-DG_DISABLE_ASSERT \
	-DNDEBUG \
	-DNO_DEBUG
CXXFLAGS_OPTIMIZE = \
	-march=native \
	-std=c++14 \
	-Wno-c++98-compat \
	-O2 \
	-ffast-math \
	-finline-functions \
	-fmerge-all-constants \
	-fno-common \
	-fno-ident \
	-fomit-frame-pointer \
	-fvisibility-inlines-hidden \
	-g0
CXXFLAGS_OPTIMIZE_GNU = \
	-fdevirtualize-speculatively \
	-fdirectives-only \
	-fgcse-after-reload \
	-fgcse-las \
	-fgcse-sm \
	-fgraphite \
	-fgraphite-identity \
	-fipa-icf \
	-fipa-pta \
	-fipa-ra \
	-fira-loop-pressure \
	-fivopts \
	-floop-block \
	-floop-interchange \
	-floop-strip-mine \
	-fno-enforce-eh-specs \
	-fno-fat-lto-objects \
	-fno-semantic-interposition \
	-fnothrow-opt \
	-fpredictive-commoning \
	-frename-registers \
	-freorder-functions \
	-ftree-loop-distribute-patterns \
	-ftree-loop-distribution \
	-ftree-loop-im \
	-ftree-partial-pre \
	-ftree-switch-conversion \
	-fvect-cost-model \
	-fweb \
	-fwhole-program
CXXFLAGS_OPTIMIZE_CLANG =
LDFLAGS_OPTIMIZE = \
	-Wl,--as-needed \
	-Wl,--enable-new-dtags \
	-Wl,--hash-style=gnu \
	-Wl,--relax \
	-Wl,--sort-common \
	-Wl,-O9 \
	-Wl,-z,combreloc \
	-Wl,-z,noexecstack \
	-Wl,-z,now \
	-Wl,-z,relro \
	-flto \
	-fno-rtti \
	-fno-threadsafe-statics \
	-ftree-vectorize \
	-use-linker-plugin
LDFLAGS_OPTIMIZE_GNU = \
	-flto-odr-type-merging \
	-flto-partition=none
LDFLAGS_OPTIMIZE_CLANG =
#	-emit-llvm
CXXFLAGS_WARN = \
	-fdiagnostics-color=always \
	-fno-common \
	-funsigned-char \
	-pedantic \
	-Wabi \
	-Wall \
	-Wcast-align \
	-Wcast-qual \
	-Wcomment \
	-Wconversion \
	-Wctor-dtor-privacy \
	-Wdelete-non-virtual-dtor \
	-Wdisabled-optimization \
	-Wextra \
	-Wfloat-equal \
	-Wformat=2 \
	-Winit-self \
	-Winline \
	-Wint-to-pointer-cast \
	-Winvalid-pch \
	-Wmissing-declarations \
	-Wmissing-field-initializers \
	-Wmissing-format-attribute \
	-Wmissing-include-dirs \
	-Wmissing-noreturn \
	-Wnull-dereference \
	-Wold-style-cast \
	-Woverloaded-virtual \
	-Wpacked \
	-Wpedantic \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wshadow \
	-Wshift-negative-value \
	-Wshift-overflow \
	-Wsign-compare \
	-Wsign-conversion \
	-Wsign-promo \
	-Wsizeof-array-argument \
	-Wstrict-aliasing=1 \
	-Wstrict-overflow=5 \
	-Wswitch-bool \
	-Wswitch-default \
	-Wswitch-enum \
	-Wundef \
	-Wunreachable-code \
	-Wunused \
	-Wunused-parameter \
	-Wunused-parameter \
	-Wunused-variable \
	-Wvla \
	-Wvolatile-register-var \
	-Wwrite-strings \
	-Wno-padded \
	-Wno-weak-vtables
#	-Waggregate-return
#	-Weffc++
CXXFLAGS_WARN_GNU = \
	-fconcepts \
	-fdirectives-only \
	-ftracer \
	-Wbool-compare \
	-Wduplicated-cond \
	-Wlogical-op \
	-Wnoexcept \
	-Wnormalized=nfkc \
	-Wshift-overflow=2 \
	-Wstrict-null-sentinel \
	-Wunsafe-loop-optimizations \
	-Wunused-but-set-parameter \
	-Wunused-but-set-variable \
	-Wzero-as-null-pointer-constant
#	-Wsuggest-attribute=const
#	-Wsuggest-attribute=noreturn
#	-Wsuggest-attribute=pure
CXXFLAGS_WARN_CLANG = \
	-WNSObject-attribute \
	-Weverything \
	-Wstring-plus-int
#	-Wlogical-not-parantheses
#	-Wodr-type-mismatch
LDFLAGS_WARN = \
	-Wl,--no-allow-shlib-undefined \
	-Wl,--no-undefined \
	-Wl,--no-undefined-version \
	-Wl,--warn-common \
	-Wl,-z,defs
LDFLAGS_WARN_GNU =
LDFLAGS_WARN_CLANG =

ifdef CLANG
CXX ::= $(shell PATH=$${PATH-}$${PATH:+:}/usr/lib/llvm/*/bin \
	command -v clang++ 2>/dev/null || echo $(CXX))
CXXFLAGS_OPTIMIZE_COMPILER = $(CXXFLAGS_OPTIMIZE_CLANG)
LDFLAGS_OPTIMIZE_COMPILER = $(LDFLAGS_OPTIMIZE_CLANG)
CXXFLAGS_WARN_COMPILER = $(CXXFLAGS_WARN_CLANG)
LDFLAGS_WARN_COMPILER = $(LDFLAGS_WARN_CLANG)
else
# Assume GNU
CXXFLAGS_OPTIMIZE_COMPILER = $(CXXFLAGS_OPTIMIZE_GNU)
LDFLAGS_OPTIMIZE_COMPILER = $(LDFLAGS_OPTIMIZE_GNU)
CXXFLAGS_WARN_COMPILER = $(CXXFLAGS_WARN_GNU)
LDFLAGS_WARN_COMPILER = $(LDFLAGS_WARN_GNU)
endif

ifdef OPTIMIZE
CPPFLAGS_EXTRA ::= $(CPPFLAGS_EXTRA) \
	$(CPPFLAGS_OPTIMIZE)
CXXFLAGS_EXTRA ::= $(CXXFLAGS_EXTRA) \
	$(CXXFLAGS_OPTIMIZE) \
	$(CXXFLAGS_OPTIMIZE_COMPILER)
LDFLAGS_EXTRA ::= $(LDFLAGS_EXTRA) \
	$(LDFLAGS_OPTIMIZE) \
	$(LDFLAGS_OPTIMIZE_COMPILER)
endif

ifdef WARN
LDFLAGS_EXTRA ::= $(LDFLAGS_EXTRA) \
	$(LDFLAGS_WARN) \
	$(LDFLAGS_WARN_COMPILER)
CXXFLAGS_EXTRA ::= $(CXXFLAGS_EXTRA) \
	$(CXXFLAGS_WARN) \
	$(CXXFLAGS_WARN_COMPILER)
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
