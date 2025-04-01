#include "utils.h"
#include <readline/readline.h>
#include <readline/history.h>

char* read_line() {
    // Use readline to read the input with a prompt
    char* input = readline("shield > ");
    if (input && *input) {
        add_history(input);  // Add non-empty input to history
    }
    return input;
}

int execute_command(char* input) {
	if (!strcmp("quit", input)) 
		return 1;
	printf("%s\n", input);
	return 0;
}
