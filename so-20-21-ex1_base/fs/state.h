#ifndef INODES_H
#define INODES_H

#include <stdio.h>
#include <stdlib.h>
#include "../tecnicofs-api-constants.h"

/* FS root inode number */
#define FS_ROOT 0

#define FREE_INODE -1
#define INODE_TABLE_SIZE 50
#define MAX_DIR_ENTRIES 20

#define SUCCESS 0
#define FAIL -1

#define DELAY 5000

#define LOCKED 1

/*
 * Contains the name of the entry and respective i-number
 */
typedef struct dirEntry
{
	char name[MAX_FILE_NAME];
	int inumber;
} DirEntry;

/*
 * Data is either text (file) or entries (DirEntry)
 */
union Data
{
	char *fileContents;	 /* for files */
	DirEntry *dirEntries; /* for directories */
};

/*
 * I-node definition
 */
typedef struct inode_t
{
	type nodeType;
	union Data data;
	pthread_rwlock_t lock;
	int locked;
} inode_t;

void insert_delay(int cycles);
void inode_table_init();
void inode_table_destroy();
int inode_create(type nType);
int inode_delete(int inumber);
int inode_get(int inumber, type *nType, union Data *data);
int inode_set_file(int inumber, char *fileContents, int len);
int dir_reset_entry(int inumber, int sub_inumber);
int dir_add_entry(int inumber, int sub_inumber, char *sub_name);
void inode_print_tree(FILE *fp, int inumber, char *name);

/* Lock FileSystem's internal structure */
int lock_inode_wr(int inumber, int *num_locked, int *index);

/* Lock FileSystem's internal structure*/
int lock_inode_rd(int inumber, int *num_locked, int *index);

/* Unlock FileSystem's internal structure */
void unlock_inode(int inumber);

/* ReadLocks collection of inodes */
void lock_inodes_rd(int *inodes, int size);

/* WriteLocks collection of inodes */
void lock_inodes_wr(int *inodes, int size);

/* Unlocks collection of inodes */
void unlock_inodes(int *inodes, int size);

#endif /* INODES_H */
