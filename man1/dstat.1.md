---
title: DSTAT
section: 1
header: User Manual
footer: 0.7.2-pre-release
date: Mar 13 2024
---
# NAME
dstat - Quickly gather and print directory statistics.

# SYNOPSIS
**dstat** [*OPTION*]... [*DIRECTORY*]...

# DESCRIPTION
**dstat** quickly gathers "directory statistics" on one or more directories
supplied at the command line. It is most useful as a diagnostics and
"sanity checking" tool in BigData environments where file management can be
unwieldy. For example, trying to determine the number of files under a 
certain directory with **ls**(1) can take many minutes to complete when 
there are millions of files to be counted. **dstat**, on the other hand, 
reads the directory inode blocks directly, which can drastically reduce the 
amount of time, not to mention disk I/O overhead, needed in order to help 
determine file distribution.

# OPTIONS
**[DIRECTORY]**
: The path, relative or fully-qualified, of zero or more directories on which
to collect statistics. Statistics may differ by operating system or 
filesystem type but generally include:

> `DT_REG` - Regular file entires

> `DT_DIR` - Sub-directory entries

> `DT_LNK` - Symbolic links

> `DT_CHR` - Character-special files

> `DT_BLK` - Block-special files

Filesystems that support BSD extensions may also show:

> `DT_SOCK` - BSD Sockets

> `DT_FIFO` - FIFO (pipe) files

> `DT_WHT` - Union whiteout files

Any unknown file types (e.g not listed in `sys/dirent.h`) will be counted as
`DT_UNKNOWN`, "unknown".

Multiple directories may be specified (see EXAMPLES). If no directory is
specified, the current working directory is assumed.

**-C**, **---continuous**
: Continuously update `STDOUT` with statistical counts. This helps you look 
busy whilst sipping at your coffee. Implies `-L` / `--linear` output format.
Count updates are printed on a single line, updated inline, unless the `-L` /
`--linear` flag is explicitly called.

**-L**, **---linear**
: Rather than displaying a descriptive block of text with the stastical data
(the default output), show a header line followed by a single line of all
statistical data (see EXAMPLES). When explicitly specified with the `-c` /
`--continuous` option, count updates are printed on new lines.

**-c**, **---csv***
: Prints CSV-formatted output. With the `-o`/`--output` options (see below),
writes CSV output to the named output file. Has no effect without `-o` flag
if either `-c` or `-L` flags are also specified.

**-q**, **---quiet**
: Do not print the directory list in default output ("block") mode. With
`-c`, `-C`, and/or `-L` flags, only prints the accumulated statistical counts
without directory or header lines.

**-o**, **---outfile** [*OUTFILE*]
: Send the default output to the named `[OUTFILE]`. Note that this flag may
be used _in addition_ to aforementioned output format- control flags; this
implies opening `[OUTFILE]` on start, final population of `[OUTFILE]` prior to
termination, and output to `STDOUT` following the output format flags.

**-l**, **---logfile** [*LOGFILE*]
: Similar to the `-o` / `--outfile` flag, send error data to the named
`[LOGFILE]`. The main difference with specifying the `-l` / `--logfile` option 
is that any directory/ies that are not valid or cannot be examined (e.g 
because of permissions errors) are simply logged to `[LOGFILE]` and program 
execution continues. Normally, any directory access errors, along with all 
other errors, will cause the program to halt. 

**-v**, **---version**
: Prints the current software revision and exits.

**-V**, **---Version**
: Prints verbose software release information (including version tag, Git
commit ID, author, and date information) and exits.

**-h**, **---help**
: Display a short help message with usage information.

# EXAMPLES
**dstat**
: Prints directory statistics for the current working directory.

**dstat ---continuous /data /bigdata**
: Display combined stats for both the `/data` and `/bigdata` filesystems,
printing the output to a single line, inline, as it becomes available.

**dstat -C -L -q -o /tmp/dstat.out -l /tmp/dstat.log /bigdata | tee > /tmp/dsatat.csv**
: Obviously, this one is  a little more involved. In short, monitor progress
whilst saving state and not stopping on errant directory names but dutifully
logging such to the logfile (in this case, `/tmp/dstat.log`). In particular, 
continuously follow the output (via `STDOUT`) for scanning the `/bigdata`
directory. Summary output is sent to an output file, here `/tmp/dstat.out`.

**dstat --linear --csv --outfile=/tmp/outfile.csv /bigdata**
: Print statistics for the `/bigdata` filesystem as a formatted line to
`stdout` with the same information written to `/tmp/outfile.csv` in CSV format.

**dstat --quiet --csv --outfile=/tmp/outfile.csv /bigdata**
: Update the `/tmp/outfile.csv` output file with the latest statistics from the
`/bigdata` filesystem.

**dstat ---logfile /dev/null /data /bigdata**
: Print statistics on the `/data` and `/bigdata` filesystems ignoring any non-
fatal errors.

# AUTHORS
Written by Walter G Davies, A Stranger Chronicle.
<https://github.com/ASCCON/>

# COPYRIGHT
**MIT License**

_Copyright (c) 2024 Walter G Davies, A Stranger Chronicle_

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


# BUGS
- There is currently not a timestamp function for updating output files.

- Although this is most likely to be used on GNU/Linux clusters running a
BigData Platform (e.g Apache Hadoop, <https://hadoop.apache.org>), current
development is on macOS Sonoma (Darwin 23.3.0) with XCode and the LLVM CLang 
compiler. Porting it to other platforms should be relatively easy.

Submit bug reports online at: <https://github.com/ASCCON/DStat/issues>

# SEE ALSO
Full documentation and sources at: <https://github.com/ASCCON/DStat>
