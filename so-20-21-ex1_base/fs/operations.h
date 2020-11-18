#ifndef FS_H
#define FS_H

#include "state.h"

#define READ 2
#define WRITE 3
#define MAX 5
#define DELAY_SLEEP DELAY*100
#define INTRALOCK -2
#define INTERLOCK -3 


void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete (char *name);
int lookup_aux(char *name, int *locked_inodes, int parentLock, int *numLocked);
int lookup(char *name);
int move(char *source, char *destination);
void print_tecnicofs_tree(FILE *fp);
int lookup_aux_move(char *name, int *locked_inodes, int parentLock, int *numLocked);
#endif /* FS_H */
