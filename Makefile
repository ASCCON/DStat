# Developed on macOS Sonoma with XCode, CLang, and Homebrew. This should be
# relatively easy to port to native BSD, GNU/Linux, and other Unix-like
# operating systems.

NAME := dstat

SOURCE := $(NAME).c

TARGET := $(NAME)

MANPAGE := man1/$(TARGET).1

VERSION := lib/version.h

TAG := $(shell git describe --tags)

COMMIT := $(shell git log -1 HEAD | egrep '^commit' | cut -d' ' -f2)

AUTHOR := $(shell git log -1 HEAD | egrep '^Author:')

DATE := $(shell git log -1 HEAD | egrep '^Date:')

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
	$(CC) $(CFLAGS) $(LDFLAGS) $(DFLAGS) $(WFLAGS) -o $(TARGET) $(SOURCE)

$(VERSION):
	rm -f $(VERSION)
	echo "/**\n * DStat lib/version.c\n *\n * Auto-generated at build time.\n */" > $(VERSION)
	echo "const char *PROGNAME = \"$(NAME)\";" >> $(VERSION)
	echo "const char *VERSION = \"$(TAG)\";"   >> $(VERSION)
	echo "const char *COMMIT = \"$(COMMIT)\";" >> $(VERSION)
	echo "const char *AUTHOR = \"$(AUTHOR)\";" >> $(VERSION)
	echo "const char *DATE = \"$(DATE)\";"     >> $(VERSION)

$(MANPAGE):
	$(PD) man1/$(TARGET).1.md -s -t man -o man1/$(TARGET).1 

$(TARGET):
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) $(OFLAGS) $(TARGET) $(SOURCE)

all: $(VERSION) $(TARGET) $(MANPAGE)

install:
	install $(TARGET) /usr/local/bin/
	install man1/$(MANPAGE) /usr/local/man/man1/

# Don't get confused by any files actually named, 'clean'.
.PHONY: clean
clean:
	rm -f $(TARGET)

mrproper: clean
	rm -f $(VERSION) $(MANPAGE)

