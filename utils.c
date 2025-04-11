#include "utils.h"
#include "builtin.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <unistd.h>

int max_commands = 10;
char*** commands = NULL; 

void free_commands() {
	for (int i = 0; commands[i]; i++) {
		for (int j = 0; commands[i][j]; j++) {
			free(commands[i][j]);
		}
		free(commands[i]);
	}
	free(commands);
}

void set_color_red() {
	fprintf(stderr, "\033[0;31m");
}

void reset_color() {
	fprintf(stderr, "\033[0m");
}

char* read_line() {
	char* input = readline("shield > ");
	if (input && *input) {
		add_history(input);
	}
	return input;
}

char** split_line(char* input) {
	int bufsize = 64;
	int position = 0;
	char** tokens = malloc(bufsize * sizeof(char*));
	char* token;
	if (!tokens) {
		printf("shield: malloc failed\n");
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
				printf("shield: realloc failed\n");
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

	if (commands) {
		free_commands();
		commands = NULL;
	}

	for (int i = 0; args[i]; i++) {
		for (int j = 0; j < env_length; j++) {
			if (!strcmp(args[i], env[j].name)) {
				args[i] = strdup(env[j].value);
			}
		}
	}

	char* file = NULL;
	if (!strcmp(args[0], "redirect-into") && args[1]) {
		file = strdup(args[1]);
		args += 2;
	}

	commands = malloc(max_commands * sizeof(char**));
	commands[0] = malloc(64 * sizeof(char*));
	int command_count = 0;
	int arg_count = 0;

	for (int i = 0; args[i]; i++) {
		if (!strcmp(args[i], "|")) {
			commands[command_count][arg_count] = NULL;  // null-terminate the current command
			command_count++;
			arg_count = 0;

			if (command_count >= max_commands) {
				max_commands *= 2;
				commands = realloc(commands, max_commands * sizeof(char**));
			}
			commands[command_count] = malloc(64 * sizeof(char*));  // new command
			continue;
		}
		commands[command_count][arg_count++] = strdup(args[i]);
	}

	int return_value = 0;
	int found = 0;

	for (int n = 0; n <= command_count; n++) {
		if (!commands[n] || !commands[n][0]) continue;
		for (int i = 0; i < aliases_length; i++) {
			if (!strcmp(commands[n][0], aliases[i].name)) {
				found = 1;
				if (file) {
					int old_stdout = dup(STDOUT_FILENO);
					freopen(file, "w", stdout);
					return_value = execute_command(aliases[i].command);
					fflush(stdout);
					dup2(old_stdout, STDOUT_FILENO);
					close(old_stdout);
				}
				else
					return_value = execute_command(aliases[i].command);
				return return_value;
			}
		}


		for (int i = 0; i < num_builtins(); i++) {
			if (!strcmp(commands[n][0], builtins[i].name)) {
				found = 1;
				if (file) {
					int old_stdout = dup(STDOUT_FILENO);
					freopen(file, "w", stdout);
					return_value = (*builtins[i].function)(commands[n]);
					fflush(stdout);
					dup2(old_stdout, STDOUT_FILENO);
					close(old_stdout);
				}
				else
					return_value = (*builtins[i].function)(commands[n]);
				break;
			}
		}
		if (!found)
			printf("shield: %s command is not found.\n", args[0]);
	}
	
	return return_value;
}

int load_env() {
	FILE* fp = fopen("shield.env", "r");
	if (fp == NULL) {
		set_color_red();
		fprintf(stdout, "shield: couldn't open \"shield.env\", env variables are not loaded!\n");
		reset_color();
		fp = fopen("shield.env", "w");
		if (fp) fclose(fp);
		return 1;
	}
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	env = NULL;
	int idx = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[read - 1] == '\n') line[read - 1] = '\0';
		char* name = strtok(line,"=");
		char* value = strtok(NULL, "");

		if (!name || !value) {
			set_color_red();
			fprintf(stderr, "shield: invalid env variable at line %d\n", idx + 1);
			reset_color();
			continue;
		}

		env = realloc(env, sizeof(EnvVar) * (idx + 1));
		if (!env) {
			fprintf(stderr, "shield: malloc failed\n");
		}
		env[idx].name = strdup(name);
		env[idx].value = strdup(value);
		idx++;
	}
	env_length = idx;
	free(line);
	fclose(fp);
	return 0;
}

int load_aliases() {
	FILE* fp = fopen("shield.aliases", "r");
	if (fp == NULL) {
		set_color_red();
		fprintf(stdout, "shield: couldn't open \"shield.aliases\", aliases are not loaded!\n");
		reset_color();
		fp = fopen("shield.aliases", "w");
		if (fp) fclose(fp);
		return 1;
	}
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	aliases = NULL;
	int idx = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[read - 1] == '\n') line[read - 1] = '\0';
		char* name = strtok(line,"=");
		char* value = strtok(NULL, "");

		if (!name || !value) {
			set_color_red();
			fprintf(stderr, "shield: invalid alias at line %d\n", idx + 1);
			reset_color();
			continue;
		}
		
		aliases = realloc(aliases, sizeof(Alias) * (idx + 1));
		aliases[idx].name = strdup(name);

		// Tokenize the command string like in split_line
		int cmd_bufsize = 64, cmd_pos = 0;
		char** cmd_parts = malloc(cmd_bufsize * sizeof(char*));
		char* tok = strtok(value, " ");
		while (tok != NULL) {
			cmd_parts[cmd_pos++] = strdup(tok);
			if (cmd_pos >= cmd_bufsize) {
				cmd_bufsize *= 2;
				cmd_parts = realloc(cmd_parts, cmd_bufsize * sizeof(char*));
			}
			tok = strtok(NULL, " ");
		}
		cmd_parts[cmd_pos] = NULL;
		aliases[idx].command = cmd_parts;

		idx++;
	}
	aliases_length = idx;
	free(line);
	fclose(fp);
	return 0;
}
