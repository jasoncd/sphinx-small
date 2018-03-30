/**
 * mdef_convert.c - convert text to binary model definition files (and vice versa)
 *
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 **/

#include <stdio.h>
#include <string.h>

#include <pocketsphinx.h>

#include "bin_mdef.h"

int main(int argc, char *argv[])
{
    const char *infile, *outfile;
    bin_mdef_t *bin;
    int tobin = 1;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s [-text | -bin] INPUT OUTPUT\n",
                argv[0]);
        return 1;
    }
    if (argv[1][0] == '-') {
        if (strcmp(argv[1], "-text") == 0) {
            tobin = 0;
            ++argv;
        }
        else if (strcmp(argv[1], "-bin") == 0) {
            tobin = 1;
            ++argv;
        }
        else {
            fprintf(stderr, "Unknown argument %s\n", argv[1]);
            fprintf(stderr, "Usage: %s [-text | -bin] INPUT OUTPUT\n",
                    argv[0]);
            return 1;
        }
    }
    infile = argv[1];
    outfile = argv[2];

    if (tobin) {
        if ((bin = bin_mdef_read_text(NULL, infile)) == NULL) {
            fprintf(stderr, "Failed to read text mdef from %s\n", infile);
            return 1;
        }
        if (bin_mdef_write(bin, outfile) < 0) {
            fprintf(stderr, "Failed to write binary mdef to %s\n",
                    outfile);
            return 1;
        }
    }
    else {
        if ((bin = bin_mdef_read(NULL, infile)) == NULL) {
            fprintf(stderr, "Failed to read binary mdef from %s\n",
                    infile);
            return 1;
        }
        if (bin_mdef_write_text(bin, outfile) < 0) {
            fprintf(stderr, "Failed to write text mdef to %s\n", outfile);
            return 1;
        }
    }

    return 0;
}
