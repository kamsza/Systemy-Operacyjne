/*
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int MAX_ARGS = 10;



typedef struct command {
	char* name;
	char** arguments;
} command;



typedef struct commands {
	command** cmd;
	int size;
	int* comands_in_line;
} commands;





////////////////////////////////////////// LOADING DATA //////////////////////////////////////////


char** get_arguments(char* line) {
	if(line == NULL) {
		char** arguments = calloc(1, sizeof(char*));
		arguments[0] = NULL;
		return arguments;
	}

	int arguments_num = 1;
	char* copy = calloc(strlen(line), sizeof(char));
	strcpy(copy, line);

	char tokens[3] = {' ', '\n', EOF};
	strtok(copy, tokens);
  	while (strtok(NULL,  tokens)) {
      arguments_num++;
   }

	char** arguments = calloc(arguments_num + 1, sizeof(char*));
	arguments[0] = strtok(line, "  \n");
	
	for(int i = 1; i < arguments_num; i++) 
		arguments[i] = strtok(NULL, "  \n");
	
		
	arguments[arguments_num] = NULL;

	return arguments;
}



int count_commands(char *line) {
	int commands_num = 1;
	for(int i = 0; i < strlen(line); i++) {
		if(line[i] == '|'){
            commands_num++;
      }
	}
	return commands_num;
}



command* to_command(char* line) {
	int commands_num = count_commands(line);

	command* cmd = calloc(commands_num, sizeof(command));

	char** command = calloc(commands_num, sizeof(char*));
	char* com = strtok(line, "|\n");

	for(int i = 0; i < commands_num; i++) {
		command[i] = com;
		com = strtok(NULL, "|\n");
	}
	if(com != NULL) {
		printf("parsing char* to command unsuccesfull");
		return NULL;
	}


	for(int i = 0; i < commands_num; i++) {
		cmd[i].arguments = get_arguments(command[i]); 
		cmd[i].name = strtok(command[i], " ");
	}

	return cmd;
}







commands* read_file(char* file_name) {
	FILE* file = fopen(file_name, "r");

	if(!file) {
		printf("Unable to open file: %s", file_name);
		return NULL;
	}

	fseek(file, 0L, SEEK_END);
	int file_size = ftell(file);
   fseek(file, 0L, SEEK_SET);

	if(file_size == -1) { printf("Error occured while using ftell"); return NULL; }
	if(file_size == 0) { printf("File: %s is empty", file_name); return NULL; }

	char* file_content = calloc(file_size + 1, sizeof(char));
	int read_el = fread(file_content, 1, file_size, file);
	if(read_el != file_size) {
		printf("Couldn't read %s content", file_name);
		return NULL;
	}
	fclose(file);


	int num_of_commands = 0;
	for(int i = 0; i < strlen(file_content); i++) 
		if(file_content[i] == '\n')
            num_of_commands ++;
        
	

	char**lines = calloc(num_of_commands, sizeof(char*));
	if(!lines) {
		printf("calloc failure\n");
		return NULL;
	}

	char* line = strtok(file_content, "\n");
	for(int i = 0; i < num_of_commands; i++) {
		lines[i] = line;
		line = strtok(NULL, "\n");
	}
	if(line) {
		printf("Error while reading lines with commands");
		return NULL;
	}



	commands *new_cmds = malloc(sizeof(commands));
	new_cmds->cmd = calloc(num_of_commands, sizeof(command*));

	if(!new_cmds || !new_cmds->cmd) {
		printf("Error in memory allocation");
		return NULL;
	}

	new_cmds->size = num_of_commands;
	new_cmds->comands_in_line = calloc(num_of_commands, sizeof(int));

	for(int line = 0; line < num_of_commands; line++)
		new_cmds->comands_in_line[line] = count_commands(lines[line]);

	for(int line = 0; line < num_of_commands; line++)	
		new_cmds->cmd[line] = to_command(lines[line]);

	return new_cmds;
}






//////////////////////////////////////// EXECUTING COMMANDS ////////////////////////////////////////


void execute(commands* cmds) {
	int curr_pipe[2];
	int  prev_pipe[2];

	printf("\n\n----------------------------\n\n");
	
	for(int x = 0; x < cmds->size; x++) {

		command *cmd = cmds->cmd[x];


		for(int i = 0; i < cmds->comands_in_line[x]; i++) {
			if(pipe(curr_pipe) == -1) {
				printf("pipe() error");
				exit(1);
			}
		
			pid_t pid = fork();
			if(pid == 0) {
				close(curr_pipe[0]);
				if (i > 0) dup2(prev_pipe[0], STDIN_FILENO);
				if (i < cmds->comands_in_line[x] - 1) dup2(curr_pipe[1], STDOUT_FILENO);

				execvp(cmd[i].name, cmd[i].arguments);
			}

			close(prev_pipe[0]);
			close(curr_pipe[1]);
			wait(NULL);

			prev_pipe[0] = curr_pipe[0];
		}
		
		printf("\n----------------------------\n\n");
	}
}





//////////////////////////////////////////// MAIN ////////////////////////////////////////////


int main(int argc, char *argv[], char *env[]) {
	if(argc != 2) {
		printf("Wrong arguments. Expected: <FILE NAME>");
		return 1;
	}

	commands* cmds = read_file(argv[1]);
	execute(cmds);

	return 0;
}


