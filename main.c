#include "utils.h"

int main(int argc, char** argv) {
	enable_raw_mode();
	while(1) {
		print_prompt();
		char* input = read_line();
		if (input == NULL) continue;
		//execute_command(input);
		free(input);
	}
	disable_raw_mode();
}
