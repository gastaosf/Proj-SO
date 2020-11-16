#include "operations.h"
#include "state.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char *path, char **parent, char **child)
{

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len - 1] == '/')
	{
		path[len - 1] = '\0';
	}

	for (int i = 0; i < len; ++i)
	{
		if (path[i] == '/' && path[i + 1] != '\0')
		{
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0)
	{ // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;
}

/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs()
{
	inode_table_init();

	/* create root inode */
	int root = inode_create(T_DIRECTORY);
	if (root != FS_ROOT)
	{
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs()
{
	inode_table_destroy();
}

/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries)
{
	if (dirEntries == NULL)
	{
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (dirEntries[i].inumber != FREE_INODE)
		{
			return FAIL;
		}
	}
	return SUCCESS;
}

/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries)
{
	if (entries == NULL)
	{
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0)
		{
			return entries[i].inumber;
		}
	}
	return FAIL;
}

/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType)
{

	int parent_inumber = 0;
	int child_inumber = 0;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	int locked_inodes[INODE_TABLE_SIZE];
	int numLocked = 0;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	/* create node to be added */
	child_inumber = inode_create(nodeType);

	if (child_inumber == FAIL)
	{
		printf("failed to create %s in  %s, couldn't allocate inode\n",
			   child_name, parent_name);

		return FAIL;
	}

	parent_inumber = lookup_aux(parent_name, locked_inodes, WRITE, &numLocked);

	if (parent_inumber == FAIL)
	{
		printf("failed to create %s, invalid parent dir %s\n",
			   name, parent_name);
		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to create %s, parent %s is not a dir\n",
			   name, parent_name);
		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL)
	{
		printf("failed to create %s, already exists in dir %s\n",
			   child_name, parent_name);
		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	/* add entry to folder that contains new node */

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL)
	{
		printf("could not add entry %s in dir %s\n",
			   child_name, parent_name);
		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	unlock_inodes(locked_inodes, numLocked);
	return SUCCESS;
}

/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete (char *name)
{

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	int locked_inodes[INODE_TABLE_SIZE];
	int size = 0;
	int *numLocked = &size;
	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);
	parent_inumber = lookup_aux(parent_name, locked_inodes, WRITE, numLocked);

	if (parent_inumber == FAIL)
	{
		printf("failed to delete %s, invalid parent dir %s\n",
			   child_name, parent_name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to delete %s, parent %s is not a dir\n",
			   child_name, parent_name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL)
	{
		printf("could not delete %s, does not exist in dir %s\n",
			   name, parent_name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL)
	{
		printf("could not delete %s: is a directory and not empty\n",
			   name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL)
	{
		printf("failed to delete %s from dir %s\n",
			   child_name, parent_name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL)
	{
		printf("could not delete inode number %d from dir %s\n",
			   child_inumber, parent_name);
		unlock_inodes(locked_inodes, size);

		return FAIL;
	}

	unlock_inodes(locked_inodes, size);

	return SUCCESS;
}
/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * 	- locked_inodes: storage for locked_inodes
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup_aux(char *name, int *locked_inodes, int parentLock, int *numLocked)
{
	char full_path[MAX_FILE_NAME];
	char *saveptr;
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok_r(full_path, delim, &saveptr);

	/* lock root note */
	if (!path && parentLock == WRITE)
	{
		lock_inode_wr(current_inumber, locked_inodes, numLocked);
	}
	else
	{
		lock_inode_rd(current_inumber, locked_inodes, numLocked);
	}

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL)
	{
		path = strtok_r(NULL, delim, &saveptr);
		if (path || parentLock == READ)
			lock_inode_rd(current_inumber, locked_inodes, numLocked);
		else
			lock_inode_wr(current_inumber, locked_inodes, numLocked);

		inode_get(current_inumber, &nType, &data);
	}
	return current_inumber;
}

int lookup(char *name)
{
	int parent_inumber = 0;
	int num_locked = 0;
	int locked_inodes[INODE_TABLE_SIZE];
	parent_inumber = lookup_aux(name, locked_inodes, READ, &num_locked);
	unlock_inodes(locked_inodes, num_locked);
	return parent_inumber;
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp)
{
	inode_print_tree(fp, FS_ROOT, "");
}

int move(char *source, char *destination)
{
	int source_parent_inumber = 0;
	int source_child_inumber = 0;
	int dest_parent_inumber = 0;

	char *source_parent, *source_child, *dest_parent, *dest_child, name_copy[MAX_FILE_NAME];
	char name_copy2[MAX_FILE_NAME];
	/* use for copy */
	type source_pType, dest_pType;
	union Data source_pdata, dest_pdata;

	int locked_inodes[2 * INODE_TABLE_SIZE];

	int numLocked = 0;

	strcpy(name_copy, source);
	split_parent_child_from_path(name_copy, &source_parent, &source_child);

	strcpy(name_copy2, destination);
	split_parent_child_from_path(name_copy2, &dest_parent, &dest_child);

	int source_parent_inumber = lookup_aux(source_parent,locked_inodes,WRITE,&numLocked);

	if (source_parent_inumber == FAIL)
	{
		printf("failed to move %s, invalid parent dir %s\n",
			   source, destination);
		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	inode_get(dest_parent_inumber, &source_pType, &source_pdata);

	//lookup source child
	source_child_inumber = lookup_sub_node(source_child, source_pdata.dirEntries);

	if (source_child_inumber == FAIL)
	{
		printf("could not move %s, does not exist in dir %s\n",
			   source, source_parent);
		unlock_inodes(locked_inodes, numLocked);

		return FAIL;
	}

	inode_get(dest_parent_inumber, &dest_pType, &dest_pdata);

	//check if parent is a dir
	if (dest_pType != T_DIRECTORY)
	{
		printf("failed to move %s, parent %s is not a dir\n",
			   source, dest_parent);
		unlock_inodes(locked_inodes, numLocked);

		return FAIL;
	}

	//check if entry already exists in destination
	if (lookup_sub_node(dest_child, dest_pdata.dirEntries) != FAIL)
	{
		printf("failed to move %s, parent %s already has this entry.\n",
			   source, dest_parent);

		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	//check if destination parent is already full
	if (!is_dir_empty(dest_pdata.dirEntries))
	{
		printf("failed to move %s, parent %s is full.\n",
			   source, destination);

		unlock_inodes(locked_inodes, numLocked);
		return FAIL;
	}

	//both these operations are safe after the previous conditions are met
	dir_add_entry(dest_parent_inumber, source_child_inumber, dest_child);
	dir_reset_entry(source_parent_inumber, source_child_inumber);

	unlock_inodes(locked_inodes, numLocked);
	return SUCCESS;
}

