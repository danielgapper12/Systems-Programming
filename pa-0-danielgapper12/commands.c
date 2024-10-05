#include "commands.h"
#include "header.h"

// Define the global variables
NODE *root = NULL;  // Define root
NODE *cwd = NULL;   // Define cwd

// 0 is terminator for array, indicates end of array 
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit", 0}; 

NODE* initialize() {
	root = (NODE *)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = 0;
	root->child = 0;
	root->type = 'D';
	cwd = root;
	
	//printf("Filesystem initialized!\n");
    return cwd;
}

// searches through array of commands and returns the index of the user's command if found, otherwise returns -1
int find_command(char user_command[64])
{
	int i = 0;
	while (cmd[i])
	{
		if(strcmp(user_command, cmd[i]) == 0) // strcmp returns 0 when same, negative when str1 < str2, positive when st1 > str2
		{
			return i;
		}
		i++;
	}
	return -1;
}

// calls appropiate command function depending on user's input
void select_command(NODE** cwd, char pathname[128], int index)
{
	if (strcmp(cmd[index],"quit") == 0)
	{
		quit();
	} else if(strcmp(cmd[index], "mkdir") == 0)
    {
        mkdir(*cwd, pathname);
    } else if(strcmp(cmd[index], "rmdir") == 0)
    {
        rmdir(*cwd, pathname);
    } else if(strcmp(cmd[index], "pwd") == 0)
    {
        pwd(*cwd);
        printf("\n");
    } else if(strcmp(cmd[index], "cd") == 0)
    {
        cd(cwd, pathname);
    } else if(strcmp(cmd[index], "ls") == 0)
    {
        ls(*cwd, pathname);
    } else if(strcmp(cmd[index], "creat") == 0)
    {
        creat(*cwd, pathname);
    } else if(strcmp(cmd[index], "rm") == 0)
    {
        rm(*cwd, pathname);
    } else if(strcmp(cmd[index], "save") == 0)
    {
        save(root, "filesystem.txt");
    } else if(strcmp(cmd[index], "reload") == 0)
    {
        reload(&root, "ffsim_Gapper.txt");
    }
}

// recursively searches tree to check if pathname already exists, returns error message if it does
// int searchTree (NODE* current, char pathname[128])
// {
//     if(strcmp(current->name, pathname) == 0)
//     {
//         printf("ERROR: Pathname already exists.\n"); 
//         return -1;
//     } else if (current->sibling != NULL)
//     {
//         searchTree(current->sibling, pathname);
//     } else if (current->child != NULL)
//     {
//         searchTree(current->child, pathname);
//     } else{
//         printf("Pathname doesn't exist yet!\n");
//         return 1;
//     }
// }

// checks to see if pathname already exists, returns error message if it doesn't
// int searchPathname (NODE* current, char pathname[128])
// {
//     // step through tree, search tree
//     if(strcmp(current->name, pathname) == 0)
//     {
//         printf("Pathname already exists!\n"); 
//         return 1;
//     } else if (current->sibling != NULL)
//     {
//         searchPathname(current->sibling, pathname);
//     } else if (current->child != NULL)
//     {
//         searchPathname(current->child, pathname);
//     } else{
//         printf("ERROR: Pathname doesn't exist.\n");
//         return -1;
//     }
// }

// recursively searches tree to check if Node already exists, returns Node if found
NODE* findNode (NODE* current, char pathname[128])
{
    if(strcmp(current->name, pathname) == 0)
    {
        //printf("Node found!\n"); 
        return current;
    } else if (current->sibling != NULL)
    {
        findNode(current->sibling, pathname);
    } else if (current->child != NULL)
    {
        findNode(current->child, pathname);
    } else{
        printf("ERROR: Node doesn't exist.\n");
        return NULL; 
    }
}

// reload() function helper, creates new node
NODE* create_node(char *name, char type, NODE *parent) 
{
    NODE *new_node = (NODE *)malloc(sizeof(NODE));
    if (!new_node) 
    {
        printf("Memory allocation failed for node '%s'.\n", name);
        return NULL;
    }
    strcpy(new_node->name, name);
    new_node->type = type;
    new_node->parent = parent;
    new_node->child = NULL;
    new_node->sibling = NULL;
    return new_node;
}

// reload() function helper, adds node as a child or sibling in the tree
void add_node(NODE *parent, NODE *new_node) 
{
    if (parent->child == NULL) {
        parent->child = new_node;
    } else {
        NODE *sibling = parent->child;
        while (sibling->sibling != NULL) {
            sibling = sibling->sibling;
        }
        sibling->sibling = new_node;
    }
}

// command function definitions

// creates directory with specified name
void mkdir(NODE *cwd, char pathname[128])
{
    char dirname[128], basename[64];
    char *lastSlash = strrchr(pathname, '/');
    
    if (lastSlash) 
    {
        strncpy(dirname, pathname, lastSlash - pathname);
        dirname[lastSlash - pathname] = '\0';  
        strcpy(basename, lastSlash + 1);  
    } else 
    {
        strcpy(dirname, ""); 
        strcpy(basename, pathname);
    }

    NODE *parentDir;
    if (strlen(dirname) == 0) 
    {
        parentDir = cwd;
    } else if (pathname[0] == '/') 
    {
        parentDir = findNode(root, dirname);
    } else 
    {
        parentDir = findNode(cwd, dirname);
    }

    if (parentDir == NULL) 
    {
        printf("ERROR: Directory '%s' does not exist.\n", dirname);
        return;
    }

    if (parentDir->type != 'D') 
    {
        printf("ERROR: '%s' is not a directory.\n", dirname);
        return;
    }

    NODE *existingNode = findChild(parentDir, basename);
    if (existingNode != NULL) 
    {
        printf("ERROR: A file or directory named '%s' already exists.\n", basename);
        return;
    }

    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    if (!newNode) 
    {
        printf("Memory allocation failed\n");
        return;
    }

    strcpy(newNode->name, basename);
    newNode->type = 'D';  
    newNode->parent = parentDir;
    newNode->child = NULL;
    newNode->sibling = NULL;

    add_node(parentDir, newNode);

    //printf("Directory '%s' created successfully under '%s'.\n", basename, dirname[0] ? dirname : cwd->name);
}


// removes specified directory
void rmdir(NODE *cwd, char pathname[128]) 
{
    NODE *start;
    if (pathname[0] == '/') 
    {
        start = root;  
        pathname++;
    } else {
        start = cwd; 
    }

    char *token = strtok(pathname, "/");
    NODE *target = start;
    
    while (token != NULL && target != NULL) 
    {
        target = findChild(target, token);  
        token = strtok(NULL, "/");  
    }

    if (target == NULL) 
    {
        printf("ERROR: Directory '%s' does not exist.\n", pathname);
        return;
    }

    if (target->type != 'D') 
    {    
        printf("ERROR: '%s' is not a directory.\n", pathname);
        return;
    }

    if (target->child != NULL) 
    {
        printf("ERROR: Directory '%s' is not empty.\n", pathname);
        return;
    }

    NODE *parent = target->parent;
    if (parent->child == target) 
    {
        parent->child = target->sibling;
    } else 
    {
        NODE *sibling = parent->child;
        while (sibling->sibling != NULL && sibling->sibling != target) 
        {
            sibling = sibling->sibling;
        }
        if (sibling->sibling == target) 
        {
            sibling->sibling = target->sibling;
        }
    }

    free(target);

    //printf("Directory removed successfully.\n");
}


// finds child node
NODE *findChild(NODE *cwd, char *pathname) 
{
    NODE *current = cwd->child;

    while (current != NULL) 
    {
        if (strcmp(current->name, pathname) == 0) 
        {
            return current;  
        }
        current = current->sibling;  
    }
    
    return NULL;  
}

// changes current working directory to specified directory
void cd(NODE **cwd, char *pathname) 
{
    if (strcmp(pathname, "") == 0 || strcmp(pathname, " ") == 0) 
    {
        *cwd = root;  
        return;
    }

    if (strcmp(pathname, "..") == 0) 
    {
        if (*cwd == root) 
        {
            printf("ERROR: Already at the root directory. Cannot move up.\n");
        } 
        else 
        {
            *cwd = (*cwd)->parent; 
        }
        return;
    }

    NODE *start;
    if (pathname[0] == '/') 
    {
        start = root;  
        pathname++; 
    } else 
    {
        start = *cwd;  
    }

    char *token = strtok(pathname, "/");
    NODE *target = start;

    while (token != NULL && target != NULL) 
    {
        target = findChild(target, token);  
        token = strtok(NULL, "/");  
    }

    if (target == NULL) 
    {
        printf("ERROR: Directory '%s' not found.\n", pathname);
        return;
    }

    if (target->type != 'D') 
    {
        printf("ERROR: '%s' is not a directory. Cannot change to this path.\n", target->name);
        return;
    }

    *cwd = target;
}

// lists the children directory and files
void ls(NODE *cwd, char pathname[128]) 
{
    NODE *dirNode;

    if (strcmp(pathname, "") == 0) 
    {
        dirNode = cwd;  // If no pathname provided uses the current working directory
    } 
    else 
    {
        if (pathname[0] == '/') 
        {
            dirNode = findNode(root, pathname);  // Absolute path
        } else {
            dirNode = findNode(cwd, pathname);   // Relative path
        }

        if (dirNode == NULL) 
        {
            printf("ERROR: Path '%s' does not exist.\n", pathname);
            return;
        }

        if (dirNode->type != 'D') 
        {
            printf("ERROR: '%s' is not a directory.\n", pathname);
            return;
        }
    }

    if (dirNode->child == NULL) 
    {
        return;
    }

    NODE *child = dirNode->child;
    while (child != NULL) 
    {
        printf("%c       %s\n", child->type, child->name);
        child = child->sibling; 
    }
}


// prints the working directory 
void pwd(NODE* cwd)
{
    if(cwd == root)
    {
        printf("/"); 
        return;
    }

    if(cwd->parent != NULL)
    {
        pwd(cwd->parent);  // Recursively traverses to root
    }

    printf("%s/", cwd->name);
}


// create a new file
void creat(NODE *cwd, char pathname[128]) 
{
    char dirname[128], basename[64];
    char *lastSlash = strrchr(pathname, '/');
    
    if (lastSlash) 
    {
        strncpy(dirname, pathname, lastSlash - pathname);
        dirname[lastSlash - pathname] = '\0';  
        strcpy(basename, lastSlash + 1);  
    } else 
    {
        strcpy(dirname, ""); 
        strcpy(basename, pathname);
    }

    NODE *parentDir;
    if (strlen(dirname) == 0) 
    {
        parentDir = cwd;
    } else if (pathname[0] == '/') 
    {
        parentDir = findNode(root, dirname);
    } else 
    {
        parentDir = findNode(cwd, dirname);
    }

    if (parentDir == NULL) 
    {
        printf("ERROR: Directory '%s' does not exist.\n", dirname);
        return;
    }

    if (parentDir->type != 'D') 
    {
        printf("ERROR: '%s' is not a directory.\n", dirname);
        return;
    }

    NODE *existingNode = findChild(parentDir, basename);
    if (existingNode != NULL) 
    {
        printf("ERROR: A file or directory named '%s' already exists.\n", basename);
        return;
    }

    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    if (!newNode) 
    {
        printf("Memory allocation failed\n");
        return;
    }

    strcpy(newNode->name, basename);
    newNode->type = 'F';  
    newNode->parent = parentDir;
    newNode->child = NULL;  
    newNode->sibling = NULL;  

    add_node(parentDir, newNode);

    //printf("File '%s' created successfully under '%s'.\n", basename, dirname[0] ? dirname : cwd->name);
}

// removes specified file
void rm(NODE *cwd, char pathname[128]) 
{
    NODE *target = findChild(cwd, pathname);

    if (target == NULL) 
    {
        printf("ERROR: File '%s' does not exist.\n", pathname);
        return;
    }

    if (target->type != 'F') 
    {
        printf("ERROR: '%s' is not a file.\n", pathname);
        return;
    }

    NODE *parent = target->parent;
    
    if (parent->child == target) 
    {
        parent->child = target->sibling;
    } 
    else 
    {
        NODE *sibling = parent->child;
        while (sibling->sibling != NULL && sibling->sibling != target) 
        {
            sibling = sibling->sibling;
        }
        if (sibling->sibling == target) 
        {
            sibling->sibling = target->sibling;
        }
    }

    free(target);

    printf("File '%s' removed successfully.\n", pathname);
}

// Recursively writes the file tree to output file
void save_tree(FILE *file, NODE *node, char *path) 
{
    if (node == NULL) 
    {
        return;
    }

    char full_path[128];
    
    if (strcmp(node->name, "/") == 0) 
    {
        strcpy(full_path, "/");
    } else if (strcmp(path, "/") == 0) 
    {
        sprintf(full_path, "/%s", node->name);
    } else 
    {
        sprintf(full_path, "%s/%s", path, node->name);
    }

    fprintf(file, "%c %s\n", node->type, full_path);

    save_tree(file, node->child, full_path);

    save_tree(file, node->sibling, path);
}

// Saves tree to a file
void save(NODE *root, char *filename) 
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) 
    {
        printf("ERROR: Unable to open file '%s' for writing.\n", filename);
        return;
    }

    printf("Saving file system to '%s'...\n", filename);
    save_tree(file, root, "/");  

    fclose(file);
    printf("File system saved successfully to '%s'.\n", filename);
}

// Reloads filesystem tree from the saved file
void reload(NODE **root, char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("ERROR: Unable to open file '%s' for reading.\n", filename);
        return;
    }

    *root = create_node("/", 'D', NULL); 
    NODE *current_node = *root;

    char line[128];
    while (fgets(line, sizeof(line), file)) 
    {
        char type;
        char path[128];
        
        sscanf(line, "%c %s", &type, path);

        char *token = strtok(path, "/");
        current_node = *root;  

        while (token != NULL) 
        {
            NODE *child = findChild(current_node, token);
            if (child == NULL) 
            {
                NODE *new_node = create_node(token, type, current_node);
                add_node(current_node, new_node);
                child = new_node;
            }

            current_node = child;
            token = strtok(NULL, "/");
        }
    }

    fclose(file);
    printf("File system reloaded successfully from '%s'.\n", filename);
}

void quit()
{
    save(root, "ffsim_Gapper.txt");
	printf("Programming quitting...\n");
	exit(0);
}
