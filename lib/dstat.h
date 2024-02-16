/*******************************************************/
/* dstat header file. In case that wasn't obvious. O:) */
/*******************************************************/

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
struct sel_opts {
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
struct dir_ent {
    int d_fif, d_chr, d_dir, d_blk, d_reg, d_lnk, d_sok, d_wht, d_unk;
    int lim;
    char *dn[1024]; // FIX THIS!
};

/**
 * Decides whether to add an `s` to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where a letter `s` will be populated or nulled.
 */
char *ss(int *cnt, char *s);

/**
 * Test directory, identified by pointer to const char, prior to further action.
 */
bool dir_test(const char *dn);

/**
 * Print statistics on each (tested) directory.
 * Takes integer index value and struct pointer for stats.
 */
int get_dirstats(char *dir);

/**
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
int collate_output();

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int display_output();
