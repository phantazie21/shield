#include "utils.h"
#include "builtin.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>

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

	for (int i = 0; args[i]; i++) {
		for (int j = 0; j < env_length; j++) {
			if (!strcmp(args[i], env[j].name)) {
				args[i] = strdup(env[j].value);
			}
		}
	}

	for (int i = 0; i < aliases_length; i++) {
		if (!strcmp(args[0], aliases[i].name)) {
			return execute_command(aliases[i].command);
		}
	}

	for (int i = 0; i < num_builtins(); i++) {
		if (!strcmp(args[0], builtins[i].name))
			return (*builtins[i].function)(args);
	}
	printf("shield: %s command is not found.\n", args[0]);
	
	return 1;
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
