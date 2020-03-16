#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// The following variable checks whether the parent process will wait for the child to finish or not.
/*
	1: wait for child (default)
	0: don't wait for child
*/
int wait_for_child = 1;		 

/*
  Function Declarations for builtin shell commands:
 */
int shell_pwd(char **args);
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char*);
int shell_execute(char**);
int shell_launch();

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",					// <- change current directory
  "help",				// <- provide the manual of the shell
  "exit",				// <- exit the shell
  "pwd"					// <- print working directory
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit,
  &shell_pwd
};

// Returns the number of built in commands
int shell_builtins_size() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

// Prints current directory
int shell_pwd(char **args){

	char cwd[1024];
    getcwd(cwd, sizeof(cwd));

	printf("%s\n", cwd);

	return 1;
}

// Changes current directory
int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "Shell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Shell");
    }
  }
  return 1;
}

// Provides the manual for the shell
int shell_help(char **args)
{
  int i;
  printf("\t\t\t~~~ Welcome to my Simple SHELL ~~~\n");
  printf("+ The following are built in commands:\n");

  for (i = 0; i < shell_builtins_size(); i++) {
    printf(" -> %s\n", builtin_str[i]);
  }

  return 1;
}

// Exits the shell
int shell_exit(char **args)
{
  return 0;
}

void shell_loop(void){
	char *line;
	char **args;
	int status;

	do{

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

		printf("%s> ", cwd);
		line = shell_read_line();			// Retrieve the line entered by the user
		args = shell_split_line(line);		// split the line into arguments
		status = shell_execute(args);		// execute the command and return the status of execution

		free(line);
		free(args);

	}while(status);
}

// Executes the command
int shell_execute(char **args)
{

  // If nothing is entered 
  if (args[0] == NULL) {
    return 1;
  }

  // Loop through all the built in commands
  for (int i = 0; i < shell_builtins_size(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {  // <- Check if the command entered is a built in command
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args);
}

char *shell_read_line(void){
	char *line = NULL;
	ssize_t bufsize = 0;

	getline(&line, &bufsize, stdin);	// <- Retrieves the line entered by the user
	return line;
}

#define SHELL_TOKEN_BUFSIZE 64
#define SHELL_TOKEN_DELIM " \t\r\n\a"

char **shell_split_line(char *line){
	int bufsize = SHELL_TOKEN_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "Shell: allocation error.\n");
		exit(EXIT_FAILURE);
	}

	// Split the line into pieces and store each piece into var token
	token = strtok(line, SHELL_TOKEN_DELIM);

	// Loop through each piece
	while(token != NULL){
		
		// Check for special characters in the argument
		if(*token == EOF){
			exit(EXIT_SUCCESS);
		}else if(*token == '\n'){
			tokens[position] = '\0';
			wait_for_child = 1;
			return tokens;
		}else if(*token == '&'){
			wait_for_child = 0;
			tokens[position] = '\0';
		}else {
			tokens[position] = token;
			wait_for_child = 1;
		}


		position++;

		// Checks if the line entered is greater than the size of the buffer and if yes, allocate more memory
		if(position >= bufsize){
			bufsize += SHELL_TOKEN_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if(!tokens){
				fprintf(stderr, "Shell: allocation error.\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, SHELL_TOKEN_DELIM);
	}

	tokens[position] = NULL;
	return tokens;
}

int shell_launch(char **args){
	pid_t pid, wpid;
	int status;

	pid = fork();   // Create a new proccess with the fork function

	if(pid == 0){
		// Children process

		if (execvp(args[0], args) == -1) {   // <- execute the command with its arguments
		      perror("Shell");
		}

	}else if(pid < 0){
		// Error creating process
		perror("Shell");
	}else {

		// If the flag is on, then wait for the child to finish.	
		if(wait_for_child == 1){
				// Parent process
			do {
	      		wpid = waitpid(pid, &status, WUNTRACED);          // wait till the child process finishes
	    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}

	return 1;
}

int main(int argc, char **argv){

	shell_loop();

	return 0;
}
