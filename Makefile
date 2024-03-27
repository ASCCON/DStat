# Developed on macOS Sonoma with XCode, CLang, and Homebrew. This should be
# relatively easy to port to native BSD, GNU/Linux, and other Unix-like
# operating systems.

NAME := dstat

SOURCE := $(NAME).c

TARGET := $(NAME)

MANPAGE := docs/man1/$(TARGET).1

MANSRC := docs/man1/$(TARGET).1.md

VERSION := lib/version.h

TAG := $(shell git describe --tags)

COMMIT := $(shell git log -1 HEAD | egrep '^commit' | cut -d' ' -f2)

AUTHOR := $(shell git log -1 HEAD | egrep '^Author:')

DATE := $(shell git log -1 HEAD | egrep '^Date:')

DATEFMT := $(shell git log -1 HEAD | egrep '^Date:' | cut -d' ' -f5 -f6 -f8)

BRANCH := $(shell git branch --show-current)

CC := cc

GIT := git

PD := pandoc

DEPENDENCIES := libc

SYSROOT := $(shell xcrun --show-sdk-path)

CFLAGS := -I./lib -I$(SYSROOT)/usr/include -I/opt/homebrew/include -I/usr/local/include

DFLAGS := -DDEBUG

LDFLAGS := -lc -lcargs -L$(SYSROOT)/usr/lib -L/opt/homebrew/lib -L/usr/local/lib

OFLAGS := -O3 -o

WFLAGS := -Wall -Wextra

debug:
	$(CC) -v $(CFLAGS) $(LDFLAGS) $(DFLAGS) $(WFLAGS) -o $(TARGET) $(SOURCE)

version: $(VERSION)
$(VERSION):
	rm -f $(VERSION)
	echo "/**\n * DStat lib/version.c\n *\n * Auto-generated at build time.\n */" > $(VERSION)
	echo "const char *PROGNAME = \"$(NAME)\";" >> $(VERSION)
	echo "const char *VERSION = \"$(TAG)\";"   >> $(VERSION)
	echo "const char *COMMIT = \"$(COMMIT)\";" >> $(VERSION)
	echo "const char *AUTHOR = \"$(AUTHOR)\";" >> $(VERSION)
	echo "const char *DATE = \"$(DATE)\";"     >> $(VERSION)
	echo "const char *BRANCH = \"$(BRANCH)\";" >> $(VERSION)

manpage: $(MANPAGE)
$(MANPAGE):
	$(shell sed -i.bak -e "s/^footer: .*/footer: $(TAG)/g" $(MANSRC))
	$(shell sed -i.bak -e "s/^date: .*/date: $(DATEFMT)/g" $(MANSRC))
	$(PD) docs/man1/$(TARGET).1.md -s -t man -o $(MANPAGE)
	cp $(MANSRC) README.md
	$(shell sed -i.bak -e "s/\*\*---/\*\*--/g" README.md)
	rm -f README.md.bak
	rm -f $(MANSRC).bak

$(TARGET):
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) $(OFLAGS) $(TARGET) $(SOURCE)

all: $(VERSION) $(MANPAGE) $(TARGET)

install:
	install $(TARGET) /usr/local/bin/
	install docs/man1/$(MANPAGE) /usr/local/man/man1/

# Don't get confused by any files actually named, 'clean'.
.PHONY: clean
clean:
	rm -f $(TARGET)

mrproper: clean
	rm -f $(VERSION) $(MANPAGE)

