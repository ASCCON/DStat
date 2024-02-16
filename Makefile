# NOTE: The dls.c and fls.c source files are kept here for historic purposes
#       only. The dstat.c file is the only current file.

SOURCE := dstat.c

TARGET := dstat

MANPAGE := man1/$(TARGET).1

CC := cc

PD := pandoc

DEPENDENCIES := libc

SYSROOT := $(shell xcrun --show-sdk-path)

CFLAGS := -I./lib -I$(SYSROOT)/usr/include -I/opt/homebrew/include -I/usr/local/include

DFLAGS := -DDEBUG

LDFLAGS := -lc -lcargs -L$(SYSROOT)/usr/lib -L/opt/homebrew/lib -L/usr/local/lib

OFLAGS := -O0 -o

WFLAGS := -Wall -Wextra

debug:
	$(CC) $(CFLAGS) $(LDFLAGS) $(DFLAGS) $(WFLAGS) -o $(TARGET) $(SOURCE)

manpage:
	$(PD) man1/$(TARGET).1.md -s -t man -o man1/$(TARGET).1 

all: $(TARGET) $(MANPAGE)

$(TARGET):
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) $(OFLAGS) $(TARGET) $(SOURCE)

install:
	install $(TARGET) /usr/local/bin/
	install man1/$(MANPAGE) /usr/local/man/man1/

clean:
	rm -f $(TARGET)
	rm -f man1/$(TARGET).1
