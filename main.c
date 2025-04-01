#include "utils.h"

int main(int argc, char** argv) {
    while (1) {
        char* input = read_line();
        if (input == NULL) continue;

	int command_return = execute_command(input);
        if (command_return)
		return command_return;
        
        free(input);
    }
}
