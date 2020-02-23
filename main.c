#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char*);
int shell_execute(char**);
int shell_launch();

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("\t\t\t~~~ Welcome to the KUKA SHELL ~~~\n");
  printf("+ Type program names and arguments, and hit enter.\n");
  printf("+ The following are built in commands:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf(" -> %s\n", builtin_str[i]);
  }

  printf("+ Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
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
		line = shell_read_line();
		args = shell_split_line(line);
		status = shell_execute(args);

		free(line);
		free(args);

	}while(status);
}

int shell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {  // <- Check if the command entered is a built in command
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args);
}

char *shell_read_line(void){
	char *line = NULL;
	ssize_t bufsize = 0;

	getline(&line, &bufsize, stdin);
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

	token = strtok(line, SHELL_TOKEN_DELIM);

	while(token != NULL){
		tokens[position] = token;
		position++;

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
		// Parent process
		do {
      		wpid = waitpid(pid, &status, WUNTRACED);          // wait till the child process finishes
    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

	}

	return 1;
}

int main(int argc, char **argv){
	
	// Load config files

	// Run command loop
	shell_loop();

	// Perform any shutdown/cleanup

	return 0;
}
