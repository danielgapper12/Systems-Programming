#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>            
#include <stdlib.h>     
#include <string.h>   

#define MAX_INPUT 256

typedef struct node {
	char  name[64];       // node's name string
	char  type;			  // Type is D for directories, F for files
	struct node *child, *sibling, *parent;
} NODE;

extern NODE *root; 
extern NODE *cwd;
extern char *cmd[];

#endif