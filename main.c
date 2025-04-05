#include "utils.h"

int main(int argc, char** argv) {
	int status = load_env();
	if (status == -1)
		return status;
	status = load_aliases();
	if (status == -1)
		return status;
	status = 0;
	while (status != -1) {
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
