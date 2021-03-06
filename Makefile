SHELL = /bin/sh

# clear, then define suffix list
.SUFFIXES:
.SUFFIXES: .c .h .o

CC = gcc
AR = ar

# OPTCFLAGS is to be passed to make command to add some extra flags
CFLAGS =-pedantic -Wall -Werror -Wextra -ggdb -D_XOPEN_SOURCE\
$(OPTCFLAGS)# -DNDEBUG


# these options are mandatory, do not let the 'make user' suppress them
# "-lz is for zlib"
override CFLAGS += -std=c99 -lz

# library {source,header,object} files
LSRC = $(wildcard lib/*.c)
LHDR = $(wildcard lib/*.h)
LOBJ = $(patsubst lib/%.c,obj/%.o,$(LSRC))

# program source
PSRC = $(wildcard src/*.c)
POBJ = $(patsubst %.c,%.o,$(PSRC))


LIBNAME = rapm
# library target
LTARG = build/lib$(LIBNAME).a
# program target
PTARG = bin/prog

RST_FILES = $(wildcard *.rst)
HTML_FILES = $(patsubst %.rst,%.html,$(RST_FILES))
PDF_FILES = $(patsubst %.rst,%.pdf,$(RST_FILES))

VERSION = alfa
DISTDIR = librapm-$(VERSION)
# all files important for distribution
DISTFILES = lib/ LICENCE Makefile README samples/ src/ TODO

.PHONY: all
all   : $(PTARG)


$(PTARG): bin $(LTARG) $(POBJ)
	$(CC) $(CFLAGS) $(POBJ) -Lbuild -l$(LIBNAME) -o $@


$(LTARG): build $(LOBJ)
	$(AR) -cru $@ $(LOBJ)


# define new rule with higher priority compared to implicit %.c rule

# "obj" must be "order-only prerequisite", otherwise "obj"
# causes all source files to be recompiled over and over again

# This way every source file depends on all header files
# (there is more proper, but quite annoying solution:
# gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html)
obj/%.o: lib/%.c $(LHDR) | obj
	$(CC) $(CFLAGS) -c -o $@ $<


# directory creation, these rules have no prerequisites
obj       : ; @mkdir obj
bin       : ; @mkdir bin
build     : ; @mkdir build
$(DISTDIR): ; @mkdir $(DISTDIR)

# create gzip archive
dist: $(DISTDIR)
	@cp -r $(DISTFILES) $(DISTDIR)
	tar -czf $(DISTDIR).tar.gz $(DISTDIR)
	@rm -rf $(DISTDIR)

html:	$(HTML_FILES)

pdf:	$(PDF_FILES)

%.pdf: %.rst
	rst2pdf $(<)

%.html: %.rst
	rst2html2 --report=3 $(<) > $(@)

.PHONY: clean
clean :
	rm -rf obj/ bin/ build/
	rm -f src/main.o
	rm -f $(DISTDIR).tar.gz
	rm -f $(HTML_FILES) $(PDF_FILES)
	# delete all regular files ending in "~"
	find . -name "*~" -type f -print0 | xargs -0 /bin/rm -f
