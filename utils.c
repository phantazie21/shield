#include "utils.h"
#include "builtin.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>

char* read_line() {
    // Use readline to read the input with a prompt
    char* input = readline("shield > ");
    if (input && *input) {
        add_history(input);  // Add non-empty input to history
    }
    return input;
}

char** split_line(char* input) {
	int bufsize = 64;
	int position = 0;
	char** tokens = malloc(bufsize * sizeof(char*));
	char* token;
	if (!tokens) {
		printf("malloc failed\n");
		exit(EXIT_FAILURE);
	}
	char* delim = " \t\r\n";
	token = strtok(input, delim);
	while (token != NULL) {
		tokens[position++] = token;
		if (position >= bufsize) {
			bufsize *= 2;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				printf("realloc failed\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, delim);
	}
	tokens[position] = NULL;
	return tokens;
}

int execute_command(char** args) {
	if (args[0] == NULL)
		return 0;

	for (int i = 0; i < num_builtins(); i++) {
		if (!strcmp(args[0], builtins[i].name))
			return (*builtins[i].function)(args);
	}
	
	return 0;
}
