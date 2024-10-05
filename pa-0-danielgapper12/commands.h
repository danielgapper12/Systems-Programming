#ifndef COMMANDS_H
#define COMMANDS_H

#include "header.h"

NODE* initialize();

// helper functions
int find_command(char user_command[64]);
void select_command(NODE** cwd, char pathname[128], int index);
NODE *findChild(NODE *cwd, char *pathname);
NODE* findNode (NODE* current, char pathname[128]);
void add_node(NODE *parent, NODE *new_node);
NODE* create_node(char *name, char type, NODE *parent);

// commands
void mkdir(NODE *cwd, char pathname[128]);
void rmdir(NODE *cwd, char pathname[128]);
void pwd(NODE* cwd);
void cd(NODE **cwd, char *pathname);
void ls(NODE *cwd, char pathname[128]);
void creat(NODE *cwd, char pathname[128]);
void rm(NODE *cwd, char pathname[128]);
void save(NODE *root, char *filename);
void save_tree(FILE *file, NODE *node, char *path);
void reload(NODE **root, char *filename);
void quit();

#endif
