#define _LIBUTIL_FILENAME_H_


/**
 * Returns the last part of the path, without modifying anything in memory.
 */
const char *path2basename(const char *path);

/**
 * Strip off filename from the given path and copy the directory name into dir
 * Caller must have allocated dir (hint: it's always shorter than path).
 */
void path2dirname(const char *path, char *dir);


/**
 * Strip off the smallest trailing file-extension suffix and copy
 * the rest into the given root argument.  Caller must have
 * allocated root.
 */
void strip_fileext(const char *file, char *root);

/**
 * Test whether a pathname is absolute for the current OS.
 */
int path_is_absolute(const char *file);

