#include "operations.h"
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

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name);

	if (parent_inumber == FAIL)
	{
		printf("failed to create %s, invalid parent dir %s\n",
			   name, parent_name);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to create %s, parent %s is not a dir\n",
			   name, parent_name);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL)
	{
		printf("failed to create %s, already exists in dir %s\n",
			   child_name, parent_name);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);
	if (child_inumber == FAIL)
	{
		printf("failed to create %s in  %s, couldn't allocate inode\n",
			   child_name, parent_name);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL)
	{
		printf("could not add entry %s in dir %s\n",
			   child_name, parent_name);
		return FAIL;
	}

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

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name);

	if (parent_inumber == FAIL)
	{
		printf("failed to delete %s, invalid parent dir %s\n",
			   child_name, parent_name);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to delete %s, parent %s is not a dir\n",
			   child_name, parent_name);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL)
	{
		printf("could not delete %s, does not exist in dir %s\n",
			   name, parent_name);
		return FAIL;
	}

	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL)
	{
		printf("could not delete %s: is a directory and not empty\n",
			   name);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL)
	{
		printf("failed to delete %s from dir %s\n",
			   child_name, parent_name);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL)
	{
		printf("could not delete inode number %d from dir %s\n",
			   child_inumber, parent_name);
		return FAIL;
	}

	return SUCCESS;
}

/*
 * Move source to destination
 * Input:
 *  - source: 
 *  - destination : 
 * Returns: SUCCESS or FAIL
 */
int move(char *source, char *destination)
{
	int source_parent_inumber, source_child_inumber, dest_parent_inumber;
	char *source_parent, *source_child, *dest_parent, *dest_child, name_copy[MAX_FILE_NAME], name_copy2[MAX_FILE_NAME];
	/* use for copy */
	type source_pType, dest_pType;
	union Data source_pdata, dest_pdata;

	strcpy(name_copy, source);
	split_parent_child_from_path(name_copy, &source_parent, &source_child);

	strcpy(name_copy2, destination);
	split_parent_child_from_path(name_copy2, &dest_parent, &dest_child);

	source_parent_inumber = lookup(source_parent);
	if (source_parent_inumber == FAIL)
	{
		printf("failed to move %s, invalid parent dir %s\n",
			   source, source_parent);
		return FAIL;
	}

	inode_get(source_parent_inumber, &source_pType, &source_pdata);

	if (source_pType != T_DIRECTORY)
	{
		printf("failed to move %s, parent %s is not a dir\n",
			   source, source_parent);
		return FAIL;
	}

	source_child_inumber = lookup_sub_node(source_child, source_pdata.dirEntries);

	if (source_child_inumber == FAIL)
	{
		printf("failed to move %s, invalid source %s\n",
			   source, source);
		return FAIL;

	}

	dest_parent_inumber = lookup(dest_parent);
	if (dest_parent_inumber == FAIL)
	{
		printf("failed to move %s, invalid parent dir %s\n",
			   source, dest_parent);
		return FAIL;
	}

	inode_get(dest_parent_inumber, &dest_pType, &dest_pdata);

	if(dest_pType != T_DIRECTORY){
		printf("failed to move %s, parent %s is not a dir\n",
			   source, dest_parent);
		return FAIL;
	}

	if(lookup_sub_node(dest_child,dest_pdata.dirEntries) != FAIL){
		printf("failed to move %s, parent %s already has this entry.\n",
			   source, dest_parent);
		return FAIL;
	}

	//both these operations are safe after the previous conditions are met
	dir_add_entry(dest_parent_inumber, source_child_inumber, dest_child);
	dir_reset_entry(source_parent_inumber, source_child_inumber);

	return SUCCESS;
}

/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name)
{
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok(full_path, delim);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL)
	{
		printf("%s\n", path);
		inode_get(current_inumber, &nType, &data);
		path = strtok(NULL, delim);
	}

	return current_inumber;
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
