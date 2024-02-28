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
#define dprint(fmt, ...)                                                \
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
    bool rec; // recursively descend all directories
    bool upd; // continuous update option
    bool lin; // display line output rather than descriptive block
    bool qit; // quite mode; no header lines on line output
    bool out; // send output to a file
    bool log; // send errors to a log file
    char *outfile; // name of output file
    char *logfile; // name of log file
};

/**
 * This structure holds the variables and pointers for adding dirent.h
 * statistical entries.
 */
struct dir_ent_s {
    int d_fif, d_chr, d_dir, d_blk, d_reg, d_lnk, d_sok, d_wht, d_unk;
};

/**
 * The following structs and function declarations enable the linked-list
 * for adding and traversing directory paths.
 */
/// Special type specific to directory names.
typedef char Direc;

/// A directory entry node on the linked-list.
typedef struct DirEnt {
    struct DirEnt *next;
    Direc         *dir;
} DirEnt;

/// The linked-list itself.
typedef struct {
    DirEnt *head;
    int    num_dirs;
} DirList;

/// Initialise a node on the linked-list.
DirEnt *createDirEnt(Direc *dir);

/// Initialise the linked-list in main().
DirList *createDirList();

/// Diagnostic for checking against the supplied user options.
int  numDirs(DirList *paths);

/// Add a directory entry to the linked-list.
void addDir(DirList *paths, DirEnt *dir_ent);

/// Traverse the linked-list.
void getPaths(DirList *paths, void (*printDir)(Direc *dir));

/// Get a node entry (directory path) from the linked-list.
void getDir(DirEnt *dir_ent, void (*printDir)(Direc *dir));

/**
 * Decides whether to add an `s` to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where a letter `s` will be populated or nulled.
 */
char *ess(int *cnt, char *s);

/**
 * Test directory, identified by pointer to const char, prior to further action.
 */
bool testDir(const char *dn);

/**
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
//bool collateOutput(struct dl_s *head);

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int displayOutput();
