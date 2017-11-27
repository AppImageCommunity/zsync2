
/*
 *   rcksum/lib - library for using the rsync algorithm to determine
 *               which parts of a file you have and which you need.
 *   Copyright (C) 2004,2005,2007,2009 Colin Phipps <cph@moria.org.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the Artistic License v2 (see the accompanying 
 *   file COPYING for the full license terms), or, at your option, any later 
 *   version of the same license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   COPYING file for details.
 */

/* Effectively the constructor and destructor for the rcksum object.
 * Also handles the file handles on the temporary store.
 */

#include "zsglobal.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_DMALLOC
# include <dmalloc.h>
#endif

#include "rcksum.h"
#include "internal.h"

/* rcksum_init(num_blocks, block_size, rsum_bytes, checksum_bytes, require_consecutive_matches)
 * Creates and returns an rcksum_state with the given properties
 */
struct rcksum_state *rcksum_init(zs_blockid nblocks, size_t blocksize,
                                 int rsum_bytes, int checksum_bytes,
                                 int require_consecutive_matches,
                                 char* directory) {
    /* Allocate memory for the object */
    struct rcksum_state *rs = malloc(sizeof(struct rcksum_state));
    if (rs == NULL) return NULL;

    /* Enter supplied properties. */
    rs->blocksize = blocksize;
    rs->blocks = nblocks;
    rs->rsum_a_mask = rsum_bytes < 3 ? 0 : rsum_bytes == 3 ? 0xff : 0xffff;
    rs->checksum_bytes = checksum_bytes;
    rs->seq_matches = require_consecutive_matches;

    /* require_consecutive_matches is 1 if true; and if true we need 1 block of
     * context to do block matching */
    rs->context = blocksize * require_consecutive_matches;

    /* Temporary file to hold the target file as we get blocks for it */
    static const char template[] = "rcksum-XXXXXX";
    if (directory != NULL) {strdup("rcksum-XXXXXX");
        rs->filename = (char*) calloc(strlen(directory) + strlen(template) + 2, sizeof(char));
        strcat(rs->filename, directory);
        strcat(rs->filename, "/");
        strcat(rs->filename, template);
    } else {
        rs->filename = strdup(template);
    }

    /* Initialise to 0 various state & stats */
    rs->gotblocks = 0;
    memset(&(rs->stats), 0, sizeof(rs->stats));
    rs->ranges = NULL;
    rs->numranges = 0;

    /* Hashes for looking up checksums are generated when needed.
     * So initially store NULL so we know there's nothing there yet.
     */
    rs->rsum_hash = NULL;
    rs->bithash = NULL;

    if (!(rs->blocksize & (rs->blocksize - 1)) && rs->filename != NULL
            && rs->blocks) {
        /* Create temporary file */
        rs->fd = mkstemp(rs->filename);
        if (rs->fd == -1) {
            perror("open");
        }
        else {
            {   /* Calculate bit-shift for blocksize */
                int i;
                for (i = 0; i < 32; i++)
                    if (rs->blocksize == (1u << i)) {
                        rs->blockshift = i;
                        break;
                    }
            }

            rs->blockhashes =
                malloc(sizeof(rs->blockhashes[0]) *
                        (rs->blocks + rs->seq_matches));
            if (rs->blockhashes != NULL)
                return rs;

            /* All below is error handling */
        }
    }
    free(rs->filename);
    free(rs);
    return NULL;
}

/* rcksum_filename(self)
 * Returns temporary filename to caller as malloced string.
 * Ownership of the file passes to the caller - the function returns NULL if
 * called again, and it is up to the caller to deal with the file. */
char *rcksum_filename(struct rcksum_state *rs) {
    char *p = rs->filename;
    rs->filename = NULL;
    return p;
}

/* rcksum_filehandle(self)
 * Returns the filehandle for the temporary file.
 * Ownership of the handle passes to the caller - the function returns -1 if
 * called again, and it is up to the caller to close it. */
int rcksum_filehandle(struct rcksum_state *rs) {
    int h = rs->fd;
    rs->fd = -1;
    return h;
}

/* rcksum_end - destructor */
void rcksum_end(struct rcksum_state *z) {
    /* Free temporary file resources */
    if (z->fd != -1)
        close(z->fd);
    if (z->filename) {
        unlink(z->filename);
        free(z->filename);
    }

    /* Free other allocated memory */
    free(z->rsum_hash);
    free(z->blockhashes);
    free(z->bithash);
    free(z->ranges);            // Should be NULL already
#ifdef DEBUG
    fprintf(stderr, "hashhit %d, weakhit %d, checksummed %d, stronghit %d\n",
            z->stats.hashhit, z->stats.weakhit, z->stats.checksummed,
            z->stats.stronghit);
#endif
    free(z);
}
