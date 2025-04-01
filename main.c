#include "utils.h"

int main(int argc, char** argv) {
	int status = 0;
	while (!status) {
		char* input = read_line();
		if (input == NULL) continue;
		char** args = split_line(input);
		if (args == NULL) continue;
		status = execute_command(args);

		free(input);
		free(args);
	}

	return status;
}
