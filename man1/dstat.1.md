---
title: DSTAT
section: 1
header: User Manual
footer: dstat 0.5.0
date: 15 February 2024
---
# NAME
dstat - Quickly gather and print directory statistics.

# SYNOPSIS
**dstat** [*OPTION*]... [*DIRECTORY*]...

# DESCRIPTION
**dstat** quickly gathers "directory statistics" on single directories or
recursively across entire filesystems. It is most useful as a diagnostics and
"sanity checking" tool in BigData environments where file management can be
unwieldy. For example, trying to determine the number of files under a certain
directory with **ls**(1) can take many minutes to complete when there are
millions of files to be counted. **dstat**, on the other hand, reads the
directory inode blocks directly, which can drastically reduce the amount of
time, not to mention disk I/O overhead, needed in order to help determine
file distribution.

# OPTIONS
**[DIRECTORY]**
: The path, relative of full-qualified, of one or more directories on which to
collect statistics. Statistics may differ by operating system or filesystem
type but generally include:

> `DT_REG` - Regular file entires

> `DT_DIR` - Sub-directory entries

> `DT_LNK` - Symbolic links

> `DT_CHR` - Character-special files

> `DT_BLK` - Block-special files

Filesystems that support BSD extensions may also show:

> `DT_SOCK` - BSD Sockets

> `DT_FIFO` - FIFO (pipe) files

> `DT_WHT` - Union whiteout files

Any file unknown file types (e.g not listed in sys/dirent.h) will be counted
as `DT_UNKNOWN`, "unknown".

Multiple directories may be specified (see EXAMPLES). If no directory is
specified, the current working directory is assumed.

**-c**, **--continuous**
: Continuously update STDOUT with statistical counts. This helps you look busy
whilst sipping at your coffee. Implies `-L, --linear` output format.

**-L**, **--linear**
: Rather than displaying a descriptive block of text with the stastical data
(the default output), show a header line followed by a single line of all
statistical data (e.f EXAMPLES).

**-q**, **--quiet**
: Do not print the directory list in default output mode. With `-c` or `-L`
flags, does not print the header information lines. Statistics are printed as:

> `DT_TYPE`:num,[space]...

**-r**, **--recursive**
: For each valid directory supplied, get directory stats and follow sub-
directories, adding their stats to the accumulated total. 

**-o**, **--outfile** [OUTFILE]
: Send the default output to the named `[OUTFILE]`. Default output to to a
separate output file always includes directory/ies supplied and explanatory
block output text statistcs, regardless of other output format control flags.
Note that this flag may be used _in addition_ to aforementioned output format-
control flags; the implies opening `[OUTFILE]` on start, final population of
`[OUTFILE]` prior to termination, and output to `STDOUT` following the output
format flags.

**-l**, **--logfile** [LOGFILE]
: Similar to the `-o`/`--outfile` flag, send error data to the named
`[LOGFILE]`. The main difference with specifying the `-l`/`--logfile` option is
that any directory/ies that are not valid or cannot be examined (e.g because
of permissions errors) are simply logged to `[LOGFILE]` and program execution
continues. Normally, any directory access errors, along with all other errors,
causes the program to halt. 

**-h**, **--help**
: Display a short help message with usage information.

# EXAMPLES
**dstat**
: Prints directory statistics for the current working directory.

**dstat -r /data**
: Descends through the entire `/data` filesystem and prints a summary output
of all directories, files, and different file types encountered.

**dstat --recursive --continuous /data /bigdata**
: Descend through both the `/data` and `/bigdata` filesystems and print the
statisical output to `STDOUT` as it becomes available.

**dstat -c -q -o /tmp/dstat.out -l /tmp/dstat.log -r /bigdata | tee | tr [:space:] ',' > /tmp/dsatat.csv**
: Obviously, this one is  a little more involved. In short, monitor progress
whilst saving state and not stopping on errant directory names but dutifully
logging such to the logfile (in this case, `/tmp/dstat.log`). 

In particular, continuously follow the output (via `STDOUT`) of recursively
scanning the `/bigdata` filesystem. As each sub-directory yields output, send
it to a new line of `STDOUT` as well as massaging the output to CSV format and
sending **that** output to `/tmp/dstat.csv`.

# AUTHORS
Written by WGDavies, A Stranger Chronicle Consulting.
<https://github.com/ASCCON/>

# BUGS
Submit bug reports online at: <https://github.com/ASCCON/DStat/issues>

# SEE ALSO
Full documentation and sources at: <https://github.com/ASCCON/DStat>