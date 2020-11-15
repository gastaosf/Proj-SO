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
int lookup_aux(char *name, int *locked_inodes, int parentLock, int *numLocked);
int lookup(char *name);
int move(char *source, char *destination);
void print_tecnicofs_tree(FILE *fp);
int lookup_aux_move(char *source, char *dest, int *locked_inodes, int *numLocked,int *source_inumber,int *dest_inumber);

#endif /* FS_H */
