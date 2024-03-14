/*************************************************************************/
/* dstat.c source file                                                   */
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
 * Import everything else in <dstat.h>.
 */
#include "lib/dstat.h"
#include "lib/version.h"

/**
 * Comparison strings for not descending into ourselves.
 */
const char *DFS = "/";
const char *CD = ".";
const char *PD = "..";

/**
 * CArgs library struct with options and documentation.
 */
static struct cag_option options[] = {
    {.identifier = 'C',
     .access_letters = "C",
     .access_name = "continuous",
     .value_name = NULL,
     .description = "Prints updates as they are retrieved."},

    {.identifier = 'L',
     .access_letters = "L",
     .access_name = "linear",
     .value_name = NULL,
     .description = "Print linear output rather than block."},

    {.identifier = 'c',
     .access_letters = "c",
     .access_name = "csv",
     .value_name = NULL,
     .description = "Output to CSV format."},

    {.identifier = 'q',
     .access_letters = "q",
     .access_name = "quiet",
     .value_name = NULL,
     .description = "Do not print list of directories or header information."},

    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "output",
     .value_name = "OUTFILE",
     .description = "Print directory list and accumulated stats to OUTFILE."},

    {.identifier = 'l',
     .access_letters = "l",
     .access_name = "logfile",
     .value_name = "LOGFILE",
     .description = "Do not halt on non-fatal errors but log them to LOGFILE."},

    {.identifier = 'v',
     .access_letters = "v",
     .access_name = "version",
     .value_name = NULL,
     .description = "Recurse down directories and include aggregated results."},

    {.identifier = 'V',
     .access_letters = "V",
     .access_name = "Version",
     .value_name = NULL,
     .description = "Recurse down directories and include aggregated results."},

    {.identifier = 'h',
     .access_letters = "h",
     .access_name = "help",
     .description = "Prints this help message."}};

/**
 * Separate structure for passing selected options to functions.
 */
struct sel_opts_s opt = {
    // Default values for sel_opts{}.
    .upd = false, .lin = false, .csv = false,
    .qit = false, .out = false, .log = false,
    .outfile = "", .logfile = "",
    .OUTFILE = NULL, .LOGFILE = NULL,
    .FILEOPTS =  "a", // O_WRONLY | O_CREAT | O_APPEND
    .list = {}
};

/**
 * This structure holds the variables and pointers for adding dirent.h
 * statistical entries.
 */
struct dir_ent_s de = {
    // Default values for dir_ent_s{}.
    .d_fif = 0, .d_chr = 0, .d_dir = 0,
    .d_blk = 0, .d_reg = 0, .d_lnk = 0,
    .d_sok = 0, .d_wht = 0, .d_unk = 0,
    .num_hdr = 0,
    .fqdp = NULL
};

/**
 * Function to write to output opt.outfile if specified.
 */
void writeOut(char *msg)
{
    int tmp = fprintf(opt.OUTFILE, msg, sizeof(msg));

    if ( tmp < 1 ) {
        Dprint("failed writing to %s...", opt.outfile);
        logError(true, opt.outfile);
    }
}

/**
 * Logging function to print non-fatal errors to opt.logfile or fail
 * appropriately.
 */
void logError(bool fail, char *msg)
{
    char *msg_buffer = malloc(MAXPATHLEN * 2);

    asprintf(&msg_buffer, "%s", msg);
    if ( errno == 0 ) errno = EPERM;
    Dprint("errno: %d", errno);
    asprintf(&msg_buffer, "%s: %s\n", msg_buffer, strerror(errno));

    if ( opt.log ) {
        int tmp = fprintf(opt.LOGFILE, msg_buffer, sizeof(msg_buffer) < 1);
        if ( tmp < 1 ) {
            perror(opt.logfile);
            free(msg_buffer);
            exit(EXIT_FAILURE);
        }
    }

    if ( fail || ! opt.log ) {
        Dprint("%s", msg_buffer);
        printf("%s", msg_buffer);
        free(msg_buffer);
        exit(errno);
    }
}


/**
 * Initialise the linked-list for storing directory paths.
 */
dir_list_s *createDirList()
{
    dir_list_s *dir_path = (dir_list_s *)calloc(1, sizeof(dir_list_s));

    if ( dir_path ) {
        Dprint("initialised %s", "dir_path");
        return dir_path;
    } else {
        Dprint("error %d", errno);
        logError(true, "unable to allocate directory path list");
        return NULL;
    }
}

/**
 * Initialise a list node to the linked-list for storing directory paths.
 */
dir_node_s *createDirNode(dp_name *dir)
{
    dir_node_s *new_ent = (dir_node_s *)calloc(1,sizeof(dir_node_s));

    if ( new_ent ) {
        new_ent->dir = dir;
        Dprint("%s", dir);
        return new_ent;
    } else {
        Dprint("error %d", errno);
        logError(true, "unable to allocate directory entry");
        return NULL;
    }
}

/**
 * Test directory, identified by pointer to const char, prior to further action.
 */
bool testDir(char *dir)
{
    struct stat sb;
    char *fc = NULL;

    asprintf(&fc, "%c", dir[0]);

    if ( stat(dir,&sb) == 0 && S_ISDIR(sb.st_mode) ) {
        if ( strncmp(fc, DFS, 1) == 0 ) {
            /// Push any legitimate, fully-qualified directory path to a
            /// holding element to be pushed to linked-list.
            de.fqdp = dir;
            Dprint("user: %s", de.fqdp);
        } else {
            /// Fully-qualify any legitimate, relative directory path and
            /// push to holding element.
            chdir(dir);
            de.fqdp = getwd((char *)dir);
            Dprint("FQP: %s", de.fqdp);
        }

        ++de.num_dir;
        Dprint("%s %d", "TRUE", de.num_dir);
        return true;
    }

    free(fc);

    /// If neither of the above conditions holds, `*dir` is not a valid
    /// directory path.
    Dprint("%s: %s", dir, "FALSE");
    return false;
}

/**
 * Add a directory entry to the linked-list.
 */
void addDir(dir_list_s *paths, dir_node_s *dir_node, char *path_arg)
{
    if ( testDir(path_arg) ) {
        Dprint("%s", "testDir returned TRUE to addDir");
        dir_node = createDirNode(de.fqdp);
        Dprint("addDir %s", dir_node->dir);
        dir_node_s *next = paths->head;
        paths->head = dir_node;
        dir_node->next = next;
        ++(paths->num_dirs);
        Dprint("num_dirs: %d", paths->num_dirs);
    } else {
        if ( errno == 0 ) errno = ENOENT;
        logError(false, path_arg);
    }
}

/**
 * Add the stats from a node entry (directory path) to the linked-list.
 */
void getDirStats(dir_node_s *dir_node)
{
    Dprint("%s", dir_node->dir);
    DIR *dp;
    dp = opendir(dir_node->dir);
    struct dirent *ep = readdir(dp); // from sys/dirent.h

    if ( dp ) {
        // Duff's Device?
        while ( (ep = readdir(dp)) ) {
            Dprint("ep = %hhu", ep->d_type);
            switch(ep->d_type) {
            case DT_BLK:  ++(de.d_blk);
                Dprint("DT_BLK: %d", de.d_blk);  break;
            case DT_CHR:  ++(de.d_chr);
                Dprint("DT_CHR: %d", de.d_chr);  break;
            case DT_DIR:  ++(de.d_dir);
                Dprint("DT_DIR: %d", de.d_dir);  break;
            case DT_LNK:  ++(de.d_lnk);
                Dprint("DT_LNK: %d", de.d_lnk);  break;
            case DT_REG:  ++(de.d_reg);
                Dprint("DT_REG: %d", de.d_reg);  break;
            case DT_WHT:  ++(de.d_wht);
                Dprint("DT_WHT: %d", de.d_wht);  break;
            case DT_FIFO: ++(de.d_fif);
                Dprint("DT_FIFO: %d", de.d_fif); break;
            case DT_SOCK: ++(de.d_sok);
                Dprint("DT_SOCK: %d", de.d_sok); break;
            default:      ++(de.d_unk);
                Dprint("DT_UNK: %d", de.d_unk);  break;
            }
        }
    } else {
        Dprint("error %d", errno);
        logError(false, dir_node->dir);
    }

    (void)closedir(dp);
}

/**
 * Get a list of the directory path entries in the linked-list.
 */
void getPaths(dir_list_s *paths)
{
    dir_node_s *cursor = paths->head;

    while ( cursor ) {
        getDirStats(cursor);
        cursor = cursor->next;
    }
}

/**
 * Populate a `list[]` with the directories to be output.
 * `fmt` action specifies whether to:
 *       + `csv`: Print CSV format to `STDOUT`.
 *       + `reg`: Print "regular" format to `STDOUT`.
 *       + `non`: Write the list of directories to `de.list[]`.
 */
void getDirList(dir_list_s *paths, enum action fmt)
{
    dir_node_s *cursor = paths->head;
    char            *c = malloc(sizeof(char));
    int              i = 0;

    if ( fmt == csv ) {
        printf("Director%s\n", pl(&paths->num_dirs, c, rep));
    } else if ( fmt == reg ) {
        printf("Director%s:\n", pl(&paths->num_dirs, c, rep));
    }

    while ( cursor ) {
        if ( fmt == reg ) {
            printf("\t%s\n", cursor->dir);
        } else if ( fmt == csv ) {
            printf("%s\n", cursor->dir);
        } else if ( fmt == non ) {
            opt.list[i] = cursor->dir;
            ++i;
        } else {
            errno = EINVAL;
            logError(true, "fmt: incorrect parameter usage");
        }
        cursor = cursor->next;
    }
}

/**
 * Decides whether to add an "s"/"ies" to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where the appropriate character(s) will be populated or nulled.
 */
char *pl(int *cnt, char *p, enum action act)
{
    switch( *(cnt) ) {
    case 1:
        if ( act == add ) p = "";
        if ( act == rep ) p = "y";
        break;
    default:
        if ( act == add ) p = "s";
        if ( act == rep ) p = "ies";
        break;
    }

    return p;
}

/**
 * Reads the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
void blockOutput(dir_list_s *paths, enum action act)
{
    char *b = malloc((MAXPATHLEN * paths->num_dirs) + 1024);
    char *c = malloc(sizeof(char));
    int   i = 0;

    if ( ! opt.qit ) {
        getDirList(paths, non);
        asprintf(&b, "Director%s:\n", pl(&paths->num_dirs, c, rep));

        for ( i = 0 ; i < paths->num_dirs ; ++i ) {
            asprintf(&b, "%s\t%s\n", b, opt.list[i]);
        }

        asprintf(&b, "%s\nTotals:\n", b);
    }

    asprintf(&b, "%s%8d:director%s\n",               b, de.d_dir,
             pl(&de.d_dir, c, rep));
    asprintf(&b, "%s%8d:FIFO file%s\n",              b, de.d_fif,
             pl(&de.d_fif, c, add));
    asprintf(&b, "%s%8d:character special file%s\n", b, de.d_chr,
             pl(&de.d_chr, c, add));
    asprintf(&b, "%s%8d:block special file%s\n",     b, de.d_blk,
             pl(&de.d_blk, c, add));
    asprintf(&b, "%s%8d:regular file%s\n",           b, de.d_reg,
             pl(&de.d_reg, c, add));
    asprintf(&b, "%s%8d:symlink%s\n",                b, de.d_lnk,
             pl(&de.d_lnk, c, add));
    asprintf(&b, "%s%8d:socket%s\n",                 b, de.d_sok,
             pl(&de.d_sok, c, add));
    asprintf(&b, "%s%8d:union whiteout file%s\n",    b, de.d_wht,
             pl(&de.d_wht, c, add));
    asprintf(&b, "%s%8d:unknown file type%s\n",      b, de.d_unk,
             pl(&de.d_unk, c, add));

    if ( act == wrt ) {
        writeOut(b);
    } else {
        printf("%s", b);
    }

    free(b);
    free(c);
}

/**
 * Reads the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
void csvOutput(dir_list_s *paths, enum action act)
{
    int          i = 0;
    int   values[] = {de.d_reg, de.d_dir, de.d_lnk, de.d_blk, de.d_chr,
                      de.d_fif, de.d_sok, de.d_wht, de.d_unk};
    char        *c = malloc(sizeof(char));
    char *csv_list = malloc(sizeof(STAT_CSV) + 1024);

    /// Add directory list and header if not in quiet-mode.
    if ( ! opt.qit ) {
        getDirList(paths, non);
        asprintf(&csv_list, "Director%s\n", pl(&paths->num_dirs, c, rep));
        for ( i = 0 ; i < paths->num_dirs ; ++i ) {
            asprintf(&csv_list, "%s%s\n", csv_list, opt.list[i]);
        }

        for ( i = 0 ; i < de.num_hdr ; ++i ) {
            asprintf(&csv_list, "%s%s,", csv_list, STAT_CSV[i]);
        }
        asprintf(&csv_list, "%s\b \n", csv_list); /// Remove trailing comma.
    }

    /// Push the corresponding values to the memory block.
    for ( i = 0 ; i < de.num_hdr ; ++i ) {
        asprintf(&csv_list, "%s%d,", csv_list, values[i]);
    }
    asprintf(&csv_list, "%s\b \n", csv_list); /// Remove trailing comma.

    if ( act == wrt ) {
        writeOut(csv_list);
    } else {
        printf("%s", csv_list);
    }

    free(csv_list);
}

/**
 * Print decorations for linear output.
 */
void printDeco()
{
    int i = 0, j = 0;

    for ( i = 0 ; i < de.num_hdr ; ++i ) {
        printf("+");
        for ( j = 0 ; j < de.num_hdr ; ++j ) {
            printf("-");
        }
    }
    printf("+\n");
}
/**
 * Displays output in a linear, continuous, and/or CSV format.
 */
void lineOutput(dir_list_s *paths, enum action act)
{
    int i        = 0;
    int values[] = {de.d_reg, de.d_dir, de.d_lnk, de.d_blk, de.d_chr,
                    de.d_fif, de.d_sok, de.d_wht, de.d_unk};

    /// Print decoration if not in quiet-mode.
    if ( ! opt.qit ) {
        getDirList(paths, reg);

        printDeco();
        printf("|");

        for ( i = 0 ; i < de.num_hdr ; ++i ) {
            printf("%8s |", STAT_HDR[i]);
        }

        printf("\n");
        printDeco();
    }

    if ( act == cnt ) {
        dir_node_s *cursor = paths->head;
        i = 0;

        while ( cursor ) {
            getDirStats(cursor);
            int values[] = {de.d_reg, de.d_dir, de.d_lnk,
                            de.d_blk, de.d_chr, de.d_fif,
                            de.d_sok, de.d_wht, de.d_unk};
            if ( opt.lin ) {
                printf("|");
                for ( i = 0 ; i < de.num_hdr ; ++i ) {
                    printf("%8d |", values[i]);
                }
                printf("\n");
            } else {
                printf("\r|");
                for ( i = 0 ; i < de.num_hdr ; ++i ) {
                    printf("%8d |", values[i]);
                }
            }

            ++i;
            cursor = cursor->next;
        }
    } else {
        /// Print the values with decoration.
        printf("|");
        for ( i = 0 ; i < de.num_hdr ; ++i ) {
            printf("%8d |", values[i]);
        }
    }

    /// Clean up output decorations.
    if ( ! opt.lin ) printf("\n");
    if ( ! opt.qit ) printDeco();
}

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int displayOutput(dir_list_s *paths)
{
    /// Print output as appropriate to `STDOUT`.
    if ( ( opt.upd || ( opt.lin && ! opt.csv ) )
         || ( opt.lin && opt.csv && opt.out ) ) {
        if ( opt.upd ) {
            lineOutput(paths, cnt);
        } else {
            lineOutput(paths, prt);
        }
    } else if ( opt.csv && ! opt.lin && ! opt.out ) {
        csvOutput(paths, prt);
    } else {
        blockOutput(paths, prt);
    }

    /// Write to the output file in the selected format if requested.
    if ( opt.out ) {
        if ( opt.csv ) {
            csvOutput(paths, wrt);
        } else {
            blockOutput(paths, wrt);
        }
    }

    return errno;
}

/**
 * `main()`: RTFM.
 */
int main(int argc, char *argv[])
{
    /// Initialise any starting variables not set at compile-time.
    de.num_hdr = sizeof(STAT_HDR) / sizeof(STAT_HDR[0]);

    /// Initialise, read, and set the various user options.
    int param_index = 0;
    cag_option_context context;

    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);

    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
        case 'C':
            opt.upd = true;
            break;
        case 'L':
            opt.lin = true;
            break;
        case 'c':
            opt.csv = true;
            break;
        case 'q':
            opt.qit = true;
            break;
        case 'o':
            opt.out = true;
            if ( cag_option_get_value(&context) ) {
                opt.outfile = (char *)cag_option_get_value(&context);
                opt.OUTFILE = fopen(opt.outfile, opt.FILEOPTS);
                if ( ! opt.OUTFILE ) {
                    Dprint("NULL FILE *opt.outfile: %s", opt.outfile);
                    logError(true, opt.outfile);
                }
            } else {
                errno = EINVAL;
                logError(true, "-o/--outfile must supply valid OUTFILE");
            }
            break;
        case 'l':
            opt.log = true;
            if ( cag_option_get_value(&context) ) {
                opt.logfile = (char *)cag_option_get_value(&context);
                opt.LOGFILE = fopen(opt.logfile, opt.FILEOPTS);
                if ( ! opt.LOGFILE ) {
                    Dprint("*opt.logfile: %i", opt.LOGFILE->_file);
                    logError(true, opt.logfile);
                }
            } else {
                errno = EINVAL;
                logError(true, "-l/--logfile must supply valid LOGFILE");
            }
            break;
        case 'v':
            printf("%s %s\n", PROGNAME, VERSION);
            exit(EXIT_SUCCESS);
        case 'V':
            printf("%s %s\n", PROGNAME, VERSION);
            printf("Git commit ID: %s\n", COMMIT);
            printf("%s\n", AUTHOR);
            printf("%s\n", DATE);
            exit(EXIT_SUCCESS);
        case 'h':
            printf("Usage: dstat [OPTION]... DIRECTORY...\n");
            printf("Quickly gathers and reports the numbers of various file ");
            printf("types under a\ndirectory or filesystem.\n\n");
            cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
            exit(EXIT_SUCCESS);
        case '?':
            cag_option_print_error(&context, stdout);
            exit(EXIT_FAILURE);
        }
    }

    /// Initialise the variables and linked list for storing directory paths.
    dp_name    *dir_path = NULL;
    dir_node_s *dir_node = NULL;
    dir_list_s *dir_list = createDirList();
    char       *safe_dir = malloc(MAXPATHLEN);
    int          dir_cnt = 0;

    /// Loop over all non-option arguments (directory paths or junk data)
    /// and add valid paths to the linked-list.
    for ( param_index = cag_option_get_index(&context) ;
          param_index < argc ; ++param_index ) {
        Dprint("loop: %02d: param_index = %d, dir_cnt = %d",
               dir_cnt, param_index, dir_cnt);
        asprintf(&safe_dir, "%s", argv[param_index]); /// safe string handling
        if ( strcmp(CD, safe_dir) == 0 ) {
            getcwd(safe_dir, sizeof(argv[param_index]));
        }
        addDir(dir_list, dir_node, safe_dir);
        ++dir_cnt;
    }

    /// If no directory paths were supplied from the command line,
    /// add the current working directory to the linked-list.
    Dprint("dir_cnt: %d", dir_cnt);
    if ( dir_cnt == 0 ) {
        addDir(dir_list, dir_node, getcwd(dir_path, MAXPATHLEN));
        ++dir_cnt;
    }

    if ( de.num_dir != dir_list->num_dirs ) {
        Dprint("dir_cnt: %d, de.num_dir: %d, dir_list->num_dirs: %d",
               dir_cnt, de.num_dir, dir_list->num_dirs);
        errno = EIO;
        logError(true, "directory count mismatch");
    } else if ( ( dir_cnt == 1 ) && ( opt.upd ) ) {
        errno = EINVAL;
        logError(true, "continuous update requires multiple directories");
    }

    if ( ! opt.upd ) getPaths(dir_list);

    displayOutput(dir_list);
    if ( opt.out ) fclose(opt.OUTFILE);
    if ( opt.log ) fclose(opt.LOGFILE);
    exit(errno);
}
