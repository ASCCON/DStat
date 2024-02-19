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
struct sel_opts opt = {
    // Default values for sel_opts{}.
    false, false, false, false, false, false,
    "", ""
};

/**
 * This structure holds the variables and pointers for adding dirent.h
 * statistical entries.
 */
struct dir_ent de = {
    // Default values for dir_ent{}.
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    {""}
};

/**
 * Decides whether to add an `s` to indicate singular or plural on output
 * strings. Takes an `int` of how many things in question and a pointer to
 * `char` where a letter `s` will be populated or nulled.
 */
char *ss(int *cnt, char *s)
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
bool dir_test(const char *dn)
{
    struct stat sb;

    dprint("testing directory: %s ", dn);
    if (stat(dn, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        dprint("%s", "TRUE");
        return true;
    }

    dprint("%s", "FALSE");
    return false;
}

/**
 * Print statistics on each (tested) directory.
 * Takes integer index value and struct pointer for stats.
 */
int get_dirstats(int idx)
{
    DIR *dp;
    dp = opendir(de.dn[idx]);
    struct dirent *ep = readdir(dp);
    dprint("entered get_dirstats('%s', %s, %i)", de.dn[idx], ep->d_name, opt.rec);

    if ( dp != NULL ) {
        while ( (ep = readdir(dp)) ) {
            dprint("ep = %hhu", ep->d_type);
            switch(ep->d_type) {
            case DT_BLK:  ++(de.d_blk); break;
            case DT_CHR:  ++(de.d_chr); break;
            case DT_DIR:  ++(de.d_dir); break;
            case DT_LNK:  ++(de.d_lnk); break;
            case DT_REG:  ++(de.d_reg); break;
            case DT_WHT:  ++(de.d_wht); break;
            case DT_FIFO: ++(de.d_fif); break;
            case DT_SOCK: ++(de.d_sok); break;
            default:      ++(de.d_unk); break;
            }

            if ( ep->d_type == DT_DIR && opt.rec == true &&
                 strncmp(ep->d_name, CD, ep->d_namlen) != 0 &&
                 strncmp(ep->d_name, PD, ep->d_namlen) != 0 ) {
                dprint("strncmp(CD): %d, %s, %s, %d",
                       strncmp(ep->d_name, CD, ep->d_namlen),
                       ep->d_name, CD, ep->d_namlen);
                dprint("strncmp(PD): %d, %s, %s, %d",
                       strncmp(ep->d_name, PD, ep->d_namlen),
                       ep->d_name, PD, ep->d_namlen);
                dprint("rec: de.dn[0]: %s, ep->d_name: %s", de.dn[0], ep->d_name);
                dprint("calling: get_dirstats(%s, %s, %i)",
                       ep->d_name, de.dn[0], opt.rec);
            }
        }
    } else {
        perror(de.dn[idx]);
    }

    return EXIT_SUCCESS;
}

/**
 * Takes the number of input directories, a pointer to the list of
 * directories, and the directory entry statistics structure and
 * collates the statistics into a single output block.
 */
int collate_output()
{
    int idx = 0;

    for ( idx = 0 ; idx < de.lim ; ++idx ) {
        dprint("de.lim %d, idx %2d: %s", de.lim, idx, de.dn[idx]);
        get_dirstats(idx);
        dprint("de.lim %d, idx %2d: %s", de.lim, idx, de.dn[idx]);
    }

    char *s = malloc(sizeof(char) + 1);
    dprint("idx: %d, %s", idx, de.dn[0]);
    printf("'%s'", de.dn[0]);
    if ( de.lim > 1 ) {
        for ( idx = 1 ; idx < de.lim ; ++idx ) {
            printf(" + '%s'", de.dn[idx]);
        }
    }
    printf(" entries:\n");
    printf("%8d: directorie%s\n",             de.d_dir, ss(&de.d_dir, s));
    printf("%8d: FIFO file%s\n",              de.d_fif, ss(&de.d_fif, s));
    printf("%8d: character special file%s\n", de.d_chr, ss(&de.d_chr, s));
    printf("%8d: block special file%s\n",     de.d_blk, ss(&de.d_blk, s));
    printf("%8d: regular file%s\n",           de.d_reg, ss(&de.d_reg, s));
    printf("%8d: symlink%s\n",                de.d_lnk, ss(&de.d_lnk, s));
    printf("%8d: socket%s\n",                 de.d_sok, ss(&de.d_sok, s));
    printf("%8d: union whiteout file%s\n",    de.d_wht, ss(&de.d_wht, s));
    printf("%8d: unknown file type%s\n",      de.d_unk, ss(&de.d_unk, s));

    display_output();

    return EXIT_SUCCESS;
}

/**
 * Print output(s) to the requested channel(s) in the requested format(s).
 */
int display_output()
{
    dprint("%i", opt.rec);
    return 0;
}

/**
 * `main()` takes ye olde `argc`/`argv` for the directory name to stat.
 */
int main(int argc, char *argv[])
{
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

    int dircnt = 0;

    for ( param_index = cag_option_get_index(&context); param_index < argc;
         ++param_index ) {
        dprint("loop: %02d: param_index = %d, dircnt = %d",
               dircnt, param_index, dircnt);
        if ( dir_test(argv[param_index]) == true ) {
            de.dn[dircnt] = argv[param_index];
            ++dircnt;
        } else {
            perror(argv[param_index]);
            return EXIT_FAILURE;
        }
    }

    if ( dircnt == 0 ) {
        dircnt = 1;
        de.dn[0] = ".";
    }

    de.lim = dircnt;
    exit(collate_output());
}
