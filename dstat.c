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
 * Note non-standard github.com:likle/cargs.git
 */
#include "lib/dstat.h"
#include "lib/version.h"
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
 * Comparison strings for not descending into ourselves.
 */
const char *CD = ".";
const char *PD = "..";

/**
 * CArgs library struct with options and documentation.
 */
static struct cag_option options[] = {
    {.identifier = 'r',
     .access_letters = "r",
     .access_name = "recursive",
     .value_name = NULL,
     .description = "Recurse down directories and include aggregated results."},

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
    .rec = false, .upd = false, .lin = false,
    .qit = false, .out = false, .log = false,
    .outfile = "", .logfile = ""
};

/**
 * This structure holds the variables and pointers for adding dirent.h
 * statistical entries.
 */
struct dir_ent_s de = {
    // Default values for dir_ent{}.
    .d_fif = 0, .d_chr = 0, .d_dir = 0,
    .d_blk = 0, .d_reg = 0, .d_lnk = 0,
    .d_sok = 0, .d_wht = 0, .d_unk = 0
};

/**
 * Initialise a list node to the linked-list for storing directory paths.
 */
DirEnt *createDirEnt(Direc *dir)
{
    DirEnt *new_ent = (DirEnt *)calloc(1,sizeof(DirEnt));

    if ( new_ent ) {
        new_ent->dir = dir;
        dprint("%s", dir);
        return new_ent;
    } else {
        dprint("error %d", errno);
        perror("unable to allocate directory entry");
        return NULL;
    }
}

/**
 * Initialise the linked-list for storing directory paths.
 */
DirList *createDirList()
{
    DirList *dir_path = (DirList *)calloc(1, sizeof(DirList));

    if ( dir_path ) {
        dprint("initialised %s", "dir_path");
        return dir_path;
    } else {
        dprint("error %d", errno);
        perror("unable to allocate directory path list");
        return NULL;
    }
}

/**
 * Fetch the current number of directory entries from the linked-list.
 */
int numDirs(DirList *paths)
{
    return paths->num_dirs;
}

/**
 * Add a directory entry to the linked-list.
 */
void addDir(DirList *paths, DirEnt *dir_ent)
{
    dprint("addDir %s", dir_ent->dir);
    DirEnt *next = paths->head;
    paths->head = dir_ent;
    dir_ent->next = next;

    dprint("increment num_dirs from %d", paths->num_dirs);
    paths->num_dirs++;
    dprint("increment num_dirs to %d", paths->num_dirs);
}

/**
 * Get a list of the directory path entries in the linked-list.
 */
void getPaths(DirList *paths, void (*printDir)(Direc *dir))
{
    DirEnt *cursor = paths->head;

    dprint("list has %d entries", numDirs(paths));
    while ( cursor ) {
        dprint("cursor is not %s", "NULL");
        getDir(cursor, printDir);
        cursor = cursor->next;
    }
}

/**
 * Fetch one node entry (directory path) from the linked-list.
 */
void getDir(DirEnt *dir_ent, void (*printDir)(Direc *dir))
{
    dprint("getDir: %s", dir_ent->dir);
    printDir(dir_ent->dir);
}

/**
 * Ugly-ass hack for doing some weird shizznizzle.
 */
void printChar(char *c)
{
    printf("%s\n", c);
}

/**
 * Decides whether to add an `s` to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where a letter `s` will be populated or nulled.
 */
char *ess(int *cnt, char *s)
{
    switch(*(cnt)) {
    case 1:
        s = "";
        break;
    default:
        s = "s";
        break;
    }

    return s;
}

/**
 * Test directory, identified by pointer to const char, prior to further action.
 */
bool testDir(const char *dir)
{
    struct stat sb;

    dprint("testing directory: %s ", dir);
    if (stat(dir,&sb) == 0 && S_ISDIR(sb.st_mode)) {
        dprint("%s", "TRUE");
        return true;
    }

    dprint("%s","FALSE");
    return false;
}

/**
 * Populate the `dir_ent_s` struct with the info from each directory in
 * the `dl_s{}` list.
 *//*
bool getDirStats(struct dl_s *head)
{
    struct dl_s *tmp = head;

    if ( tmp ) {
        dprint("entered getDirStats(%s)", tmp->next->dir_path);
        while ( tmp->next ) {
            if ( tmp->next->dir_path ) {
                DIR *dp;
                dp = opendir(tmp->next->dir_path);
                struct dirent *ep = readdir(dp);
                dprint("entered getDirStats('%s', '%s', %i)",
                       tmp->next->dir_path, ep->d_name, opt.rec);
                if ( dp != NULL ) {
                    while ( (ep = readdir(dp)) ) {
                        dprint("ep = %hhu", ep->d_type);
                        switch(ep->d_type) {
                        case DT_BLK:  ++(de.d_blk);
                            dprint("DT_BLK: %d", de.d_blk); break;
                        case DT_CHR:  ++(de.d_chr);
                            dprint("DT_CHR: %d", de.d_chr); break;
                        case DT_DIR:  ++(de.d_dir);
                            dprint("DT_DIR: %d", de.d_dir); break;
                        case DT_LNK:  ++(de.d_lnk);
                            dprint("DT_LNK: %d", de.d_lnk); break;
                        case DT_REG:  ++(de.d_reg);
                            dprint("DT_REG: %d", de.d_reg); break;
                        case DT_WHT:  ++(de.d_wht);
                            dprint("DT_WHT: %d", de.d_wht); break;
                        case DT_FIFO: ++(de.d_fif);
                            dprint("DT_FIFO: %d", de.d_fif); break;
                        case DT_SOCK: ++(de.d_sok);
                            dprint("DT_SOCK: %d", de.d_sok); break;
                        default:      ++(de.d_unk);
                            dprint("DT_UNK: %d", de.d_unk); break;
                        }
   *//*
                        if (ep->d_type == DT_DIR && opt.rec == true &&
                            strncmp(ep->d_name, CD, ep->d_namlen) != 0 &&
                            strncmp(ep->d_name, PD, ep->d_namlen) != 0 ) {
                            dprint("strncmp(CD): %d, %s, %s, %d",
                                   strncmp(ep->d_name, CD, ep->d_namlen),
                                   ep->d_name, CD, ep->d_namlen);
                            dprint("strncmp(PD): %d, %s, %s, %d",
                                   strncmp(ep->d_name, PD, ep->d_namlen),
                                   ep->d_name, PD, ep->d_namlen);
                            dprint("rec: de.dn[0]: %s, ep->d_name: %s",
                                   de.dn[0], ep->d_name);
                            dprint("calling: getDirStats(%s, %s, %i)",
                                   ep->d_name, de.dn[0], opt.rec);
                        }
   *//*
                    }
                } else {
                    perror(tmp->next->dir_path);
                }

                (void)closedir(dp);
            }
        }

        tmp = tmp->next->next;
    } else {
        perror("unable to access directory list");
        return false;
    }

    return true;
}
*/
/**
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 *//*
bool collateOutput(struct dl_s *head)
{
    int idx = 0;

    char *s = malloc(sizeof(char) + 1);
    dprint("idx:%d, %s", idx, de.dn[0]);
    printf("'%s'", de.dn[0]);
    if ( *de.lim> 1 ) {
        for ( idx = 1 ; idx < *de.lim ; ++idx ) {
            printf(" + '%s'", de.dn[idx]);
        }
    }
    printf(" entries:\n");
    printf("%8d:directorie%s\n",             de.d_dir, ess(&de.d_dir, s));
    printf("%8d:FIFO file%s\n",              de.d_fif, ess(&de.d_fif, s));
    printf("%8d:character special file%s\n", de.d_chr, ess(&de.d_chr, s));
    printf("%8d:block special file%s\n",     de.d_blk, ess(&de.d_blk, s));
    printf("%8d:regular file%s\n",           de.d_reg, ess(&de.d_reg, s));
    printf("%8d:symlink%s\n",                de.d_lnk, ess(&de.d_lnk, s));
    printf("%8d:socket%s\n",                 de.d_sok, ess(&de.d_sok, s));
    printf("%8d:union whiteout file%s\n",    de.d_wht, ess(&de.d_wht, s));
    printf("%8d:unknown file type%s\n",      de.d_unk, ess(&de.d_unk, s));

    displayOutput();

    return true;
}
*/
/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int displayOutput()
{
    dprint("%i",opt.rec);
    return 0;
}

/**
 * `main()`: RTFM.
 */
int main(int argc, char *argv[])
{
    /// Initialise the variables and linked list for storing directory paths.
    char *this_dir = malloc(MAXPATHLEN);
    Direc *dir_path = NULL;
    DirEnt *dir_ent = NULL;
    DirList *dir_list = createDirList();

    /// Initialise, read, and set the various user options.
    int param_index = 0;
    cag_option_context context;

    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);

    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
        case 'r':
            opt.rec = true;
            break;
        case 'h':
            printf("Usage: dstat [OPTION]... DIRECTORY...\n");
            printf("Quickly gathers and reports the numbers of various file ");
            printf("types under a\ndirectory or filesystem.\n\n");
            cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
            exit(EXIT_SUCCESS);
        case 'v':
            printf("%s %s\n", PROGNAME, VERSION);
            exit(EXIT_SUCCESS);
        case 'V':
            printf("%s %s\n", PROGNAME, VERSION);
            printf("Git commit ID: %s\n", COMMIT);
            printf("%s\n", AUTHOR);
            printf("%s\n", DATE);
            exit(EXIT_SUCCESS);
        case '?':
            cag_option_print_error(&context, stdout);
            exit(EXIT_FAILURE);
        }
    }

    int dir_cnt = 0;

    for ( param_index = cag_option_get_index(&context); param_index < argc;
         ++param_index ) {
        dprint("loop: %02d: param_index = %d, dir_cnt = %d",
               dir_cnt, param_index, dir_cnt);
        if ( testDir(argv[param_index]) ) {
            this_dir = argv[param_index];
            dir_ent = createDirEnt(this_dir);
            addDir(dir_list, dir_ent);
            ++dir_cnt;
        } else {
            dprint("error %d", errno);
            perror(argv[param_index]);
            return EXIT_FAILURE;
        }
    }

    dprint("dir_cnt: %d", dir_cnt);
    if ( dir_cnt == 0 ) {
        this_dir = getcwd(dir_path, MAXPATHLEN);
        if ( testDir(this_dir) ) {
            dir_ent = createDirEnt(this_dir);
            dir_cnt = 1;
            addDir(dir_list, dir_ent);
        } else {
            dprint("error %d", errno);
            perror("unable to add current working directory");
        }
    }

    getPaths(dir_list, printChar);

    exit(errno);
}
