/*
 * filename.c -- File and path name operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sphinxbase/filename.h"

const char * path2basename(const char *path)
{
    const char *result;

    result = strrchr(path, '/');

    return (result == NULL ? path : result + 1);
}

/* Return all leading pathname components */
void path2dirname(const char *path, char *dir)
{
    size_t i, l;

    l = strlen(path);
    for (i = l - 1; (i > 0) && !(path[i] == '/'); --i);
    if (i == 0) {
        dir[0] = '.';
        dir[1] = '\0';
    } else {
        memcpy(dir, path, i);
        dir[i] = '\0';
    }
}


/* Strip off the shortest trailing .xyz suffix */
void strip_fileext(const char *path, char *root)
{
    size_t i, l;

    l = strlen(path);
    for (i = l - 1; (i > 0) && (path[i] != '.'); --i);
    if (i == 0) {
        strcpy(root, path);     /* Didn't find a . */
    } else {
        strncpy(root, path, i);
    }
}

/* Test if this path is absolute. */
int path_is_absolute(const char *path)
{
    return path[0] == '/';
}
