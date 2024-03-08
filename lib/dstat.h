/*************************************************************************/
/* lib/dstat.h: dstat.c header file                                      */
/*                                                                       */
/* dstat - Quickly gather and print directory statistics.                */
/*                                                                       */
/* See full documentation and sources at:                                */
/* <https://github.com/ASCCON/DStat>                                     */
/*                                                                       */
/* MIT License                                                           */
/*                                                                       */
/* Copyright (c) 2024 Walter G Davies, A Stranger Chronicle (ASCCON)     */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject    */
/* to the following conditions:                                          */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES       */
/* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND              */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT           */
/* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,          */
/* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING          */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR         */
/* OTHER DEALINGS IN THE SOFTWARE.                                       */
/*************************************************************************/

/**
 * All the libraries that are fit to print.
 * Note non-standard github.com:likle/cargs.git
 */
#include <cargs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Define this as the DStat header file. For when only the most verbose
 * will do.
 */
#ifndef DSTAT_H
#define DSTAT_H
#endif

/**
 * Enables the `DT_` entries in sys/dirent.h.
 */
#define _BSD_SOURCE

/**
 * Set up debug printing.
 */
#define Dprint(fmt, ...)                                                \
  do { if (DEBUG) fprintf(stderr, "<DEBUG> %s:%d:%s(): " fmt "\n",      \
                          __FILE__, __LINE__, __func__, __VA_ARGS__); } \
  while (0)

#ifdef DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

/**
 * "Standard Boolean" library required here for function declarations.
 */
#include <stdbool.h>

/**
 * For some reason, we can figure out the system-specific, but not the generic,
 * MAXPATHLEN.
 */
#define MAXPATHLEN __DARWIN_MAXPATHLEN

/**
 * Separate structure for passing selected options to functions.
 */
struct sel_opts_s {
    bool upd;       /// continuous update option
    bool lin;       /// display line output rather than descriptive block
    bool qit;       /// quite mode; no header lines on line output
    bool out;       /// send output to a file
    bool log;       /// send errors to a log file
    char *outfile;  /// name of output file
    char *logfile;  /// name of log file
    FILE *OUTFILE;  /// file descriptor for output file
    FILE *LOGFILE;  /// file descriptor for log file
    char *FILEOPTS; /// placeholder for file handling options
};

/**
 * This structure holds the variables and pointers for adding dirent.h
 * statistical entries. It can also hold the following optional or
 * temporary parameters:
 *   - `char *fqdp` : A fully-qualified directory path string for passing
 *                    to the `struct dir_node{}`.
 */
struct dir_ent_s {
    int d_fif, d_chr, d_dir, d_blk, d_reg, d_lnk, d_sok, d_wht, d_unk;
    char *fqdp;
};

/**
 * The following structs and function declarations enable the linked-list
 * for adding and traversing directory paths.
 */
/// Special type specific to directory path names.
typedef char dp_name;

/// A directory entry node on the linked-list.
typedef struct dir_node_s {
    struct dir_node_s *next;
    dp_name           *dir;
} dir_node_s;

/// The linked-list itself.
typedef struct {
    dir_node_s *head;
    int        num_dirs;
} dir_list_s;

/// Initialise a node on the linked-list.
dir_node_s *createDirNode(dp_name *dir);

/// Initialise the linked-list in main().
dir_list_s *createDirList();

/// Add a directory entry to the linked-list.
void addDir(dir_list_s *paths, dir_node_s *dir_node, char *path_arg);

/// Traverse the linked-list to populate dir_ent_s{}.
void getPaths(dir_list_s *paths);

/// Add the stats from a node entry (directory path) to the linked-list.
void getDirStats(dir_node_s *dir_node);

/**
 * The `action` enum is generic for functions needing extra direction.
 * The `add` action tells `pl()` to add an _s_ to "pluralise" a string.
 * The `rep` action tells `pl()` to replace either a _y_ (singular) or _ies_
 * (plural) as appropriate to the calling string.
 */
enum action {
    add = 0,
    rep
};

/**
 * Decides whether to add an "s"/"ies" to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where the appropriate character(s) will be populated or nulled.
 */
char *pl(int *cnt, char *s, enum action act);

/**
 * Test directory, identified by pointer to const char, prior to further action.
 */
bool testDir(char *dir);

/**
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
void blockOutput();

/**
 * Print decorations for linear output.
 */
void printDeco();

/**
 * Displays output in a linear, continuous, and/or CSV format.
 */
void lineOutput();

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int displayOutput(dir_list_s *paths);
