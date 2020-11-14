#ifndef FS_H
#define FS_H

#include "state.h"

#define READ 2
#define WRITE 3

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete (char *name);
int lookup(char *name, int *locked_inodes, int parentLock, int *numLocked);
int lookup_aux(char *name);
void print_tecnicofs_tree(FILE *fp);
void unlock_create(int parent_inumber, int child_inumber, int *locked_inodes, int size);
void unlock_delete(int parent_inumber, int *locked_inodes, int size);
int move(char *source, char *destination);

#endif /* FS_H */
