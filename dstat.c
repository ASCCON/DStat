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
const char *DFS = "/";
const char *CD = ".";
const char *PD = "..";

/**
 * CArgs library struct with options and documentation.
 */
static struct cag_option options[] = {
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
    .upd = false, .lin = false, .qit = false,
    .out = false, .log = false,
    .outfile = "", .logfile = ""
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
    .fqdp = NULL
};

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
        perror("unable to allocate directory path list");
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
        perror("unable to allocate directory entry");
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

        Dprint("%s", "TRUE");
        return true;
    }

    free(fc);

    /// If neither of the above conditions holds, `*dir` is not a valid
    /// directory path.
    Dprint("%s","FALSE");
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

        paths->num_dirs++;
        Dprint("num_dirs: %d", paths->num_dirs);
    } else {
        Dprint("errno: %d", errno);
        perror(path_arg);
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
        perror(dir_node->dir);
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
        Dprint("%s", cursor->dir);
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
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
void collateOutput()
{
    char *c = malloc(sizeof(char));

    printf("Totals:\n");
    printf("%8d:director%s\n",               de.d_dir, pl(&de.d_dir, c, rep));
    printf("%8d:FIFO file%s\n",              de.d_fif, pl(&de.d_fif, c, add));
    printf("%8d:character special file%s\n", de.d_chr, pl(&de.d_chr, c, add));
    printf("%8d:block special file%s\n",     de.d_blk, pl(&de.d_blk, c, add));
    printf("%8d:regular file%s\n",           de.d_reg, pl(&de.d_reg, c, add));
    printf("%8d:symlink%s\n",                de.d_lnk, pl(&de.d_lnk, c, add));
    printf("%8d:socket%s\n",                 de.d_sok, pl(&de.d_sok, c, add));
    printf("%8d:union whiteout file%s\n",    de.d_wht, pl(&de.d_wht, c, add));
    printf("%8d:unknown file type%s\n",      de.d_unk, pl(&de.d_unk, c, add));

    free(c);
    displayOutput();
}

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int displayOutput()
{
    return 0;
}

/**
 * `main()`: RTFM.
 */
int main(int argc, char *argv[])
{
    /// Initialise, read, and set the various user options.
    int param_index = 0;
    cag_option_context context;

    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);

    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
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

    /// Initialise the variables and linked list for storing directory paths.
    dp_name *dir_path = NULL;
    dir_node_s *dir_node = NULL;
    dir_list_s *dir_list = createDirList();
    int dir_cnt = 0;

    /// Loop over all non-option arguments (directory paths or junk data)
    /// and add valid paths to the linked-list.
    for ( param_index = cag_option_get_index(&context) ;
          param_index < argc ; ++param_index ) {
        Dprint("loop: %02d: param_index = %d, dir_cnt = %d",
               dir_cnt, param_index, dir_cnt);
        addDir(dir_list, dir_node, argv[param_index]);
        ++dir_cnt;
    }

    /// If no directory paths were supplied from the command line,
    /// add the current working directory to the linked-list.
    Dprint("dir_cnt: %d", dir_cnt);
    if ( dir_cnt == 0 ) {
        addDir(dir_list, dir_node, getcwd(dir_path, MAXPATHLEN));
    }

    getPaths(dir_list);
    collateOutput();
    exit(errno);
}
