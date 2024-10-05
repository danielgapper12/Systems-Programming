#include "commands.h"

int main() {
	NODE *cwd = initialize(); 

	while(1) 
 	{
		char input[MAX_INPUT]; // user's input 
		char user_command[64];
		char pathname[128];

		// set strings to null terminator
		user_command[0] = '\0'; 
		pathname[0] = '\0';


		printf("Enter command: ");

		if (fgets(input, MAX_INPUT, stdin) != NULL) 
        {
            sscanf(input, "%s %s", user_command, pathname);
        }

		int index = find_command(user_command);
	
		if (index != -1)
		{
			select_command(&cwd, pathname, index);
		} else{
			printf("Invalid command entered: %s\n", user_command);
		}
	}
}

