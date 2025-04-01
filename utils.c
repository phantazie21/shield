#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

void print_prompt() {
	printf("shield > ");
}

char* read_line() {
	size_t buffer_size = 128;
	size_t length = 0;
	char* ret = malloc(buffer_size);
	if (!ret) {
		perror("malloc failed");
		return NULL;
	}

	char ch;
	while ((ch = getchar()) != '\n' && ch != EOF) {
		if (ch == '\t') {
			printf("[TAB]");
			fflush(stdout);
			continue;  // Skip appending the tab character
		} else {
			putchar(ch);  // Echo the character
			fflush(stdout);
		}

		// Resize the buffer if necessary
		if (length + 1 >= buffer_size) {
			buffer_size *= 2;
			char* temp = realloc(ret, buffer_size);
			if (!temp) {
				free(ret);
				perror("realloc failed");
				return NULL;
			}
			ret = temp;
		}

		ret[length++] = ch;
	}
	printf("\n");

	ret[length] = '\0';  // Null-terminate the string
	return ret;
}

void enable_raw_mode() {
	struct termios raw;
	tcgetattr(STDIN_FILENO, &raw);  // Get current terminal attributes

	raw.c_lflag &= ~(ECHO | ICANON);  // Disable echo and canonical mode
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);  // Apply changes
}

void disable_raw_mode() {
	struct termios cooked;
	tcgetattr(STDIN_FILENO, &cooked);

	cooked.c_lflag |= (ECHO | ICANON);  // Re-enable echo and canonical mode
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
}
