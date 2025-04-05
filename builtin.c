#include "builtin.h"
#include "utils.h"
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

Alias* aliases = NULL;
int aliases_length = 0;
EnvVar* env = NULL;
int env_length = 0;

BuiltInCommand builtins[] = {
	{"cd", shield_cd, "Change directory to the specified path."},
	{"quit", shield_exit, "Exit the shell."},
	{"list", shield_list, "List directory contents."},
	{"pwd", shield_pwd, "Print working directory."},
	{"find", shield_find, "Find a file or directory by name."},
	{"help", shield_help, "Get available commands, or detailed description on given one."},
	{"clear", shield_clear, "Clear the terminal."},
	{"out", shield_out, "Output argument onto the terminal."},
	{"time", shield_time, "Print the execution time of a command."},
	{"history", shield_history, "Print the history of commands you gave the program."},
	{"get-content", shield_get_content, "Print the content of a given file."},
	{"alias", shield_alias, "Give alias for a command, or list the aliases you have given."},
	{"env", shield_env, "Print the environment variables."},
	{"setenv", shield_setenv, "Set an environment variable."},
	{"unsetenv", shield_unsetenv, "Unsets a given environment variable."}
};
const char* detailed_help[] = {
	"\tcd: Change the current working directory.\n\tUsage: cd <path>",
	"\tquit: Exit the shell.\n\tUsage: quit",
	"\tlist: List files and directories.\n\tUsage: list (path) (depth)",
	"\tpwd: Print the current working directory.\n\tUsage: pwd",
	"\tfind: Search for a file or directory.\n\tUsage: find <name> <all/single> (depth)",
	"\thelp: Get available commands supported by shield, or a detailed description on a given command (works for builtin commands only).\n\tUsage: help (command)",
	"\tclear: Clear the terminal's content.\n\tUsage: clear",
	"\tout: Output the first argument onto the terminal.\n\tUsage: out (text)",
	"\ttime: Print the execution time of a given command.\n\tUsage: time <command>",
	"\thistory: Print the history of commands you used in shield.\n\tUsage: history",
	"\tget-content: Print the content of a given file onto the terminal.\n\tUsage: get-content <file>",
	"\talias: Give an easier to remember alias for a command with arguments, or without args, list the aliases you have given.\n\tUsage: alias (name) (command) (args)",
	"\tenv: Print all the environment variables currently set in shield.\n\tUsage: env",
	"\tsetenv: Set an environment variable to a given path.\n\tUsage: setenv <name> <path>",
	"\tunsetenv: Unset an environment variable.\n\tUsage: unsetenv <name>",
	NULL
};

int list_directory(const char* path, int depth, int max_depth) {
	if (depth >= max_depth) return 1;
	DIR* dirp;
	struct dirent* dp;
	dirp = opendir(path);
	if (!dirp) {
		perror("shield: opendir call failed\n");
		return 1;
	}
	errno = 0;
	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		for (int i = 0; i < depth; i++) {
			printf("  ");
		}
		if (dp->d_type == DT_DIR)
			printf("/ %s\n", dp->d_name);
		else
			printf("- %s\n", dp->d_name);

		if (dp->d_type == DT_DIR) {
			char full_path[1024];
			snprintf(full_path, sizeof(full_path), "%s/%s", path, dp->d_name);
			list_directory(full_path, depth + 1, max_depth);
		}
        }
	if (errno) {
		perror("shield: readdir call failed\n");
		closedir(dirp);
		return 1;
	}

	closedir(dirp);
	return 0;
}

int find(char* path, char* name, int depth, int max_depth, bool single) {
	if (max_depth != 0 && depth >= max_depth) return 1;
	DIR* dirp;
	struct dirent* dp;
	dirp = opendir(path);
	if (!dirp) {
		perror("shield: opendir call failed\n");
		return 1;
	}
	errno = 0;
	bool found = false;
	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		char full_path[1024];
		snprintf(full_path, sizeof(full_path), "%s/%s", path, dp->d_name);

		if (!strcmp(dp->d_name, name)) {
			printf("FOUND - %s\n", full_path);
			found = true;
			if (single) {
				break;
			}
		}

		// Recurse into directories
		if (dp->d_type == DT_DIR) {
			if (!find(full_path, name, depth + 1, max_depth, single)) {
				found = true;
				if (single) {
					break;
				}
			}
		}
        }
	if (errno) {
		perror("shield: readdir call failed\n");
		closedir(dirp);
		return 1;
	}

	closedir(dirp);
	return found ? 0 : 1;
}

int shield_list(char** args) {
	char* path = ".";
	int max_depth = 1;
	if (args[1] != NULL) {
		struct stat statbuf;
		if (!stat(args[1], &statbuf) && S_ISDIR(statbuf.st_mode)) {
			path = args[1];
			if (args[2] != NULL) {
				max_depth = atoi(args[2]);
				if (max_depth <= 0) {
					fprintf(stderr, "shield: invalid depth value %s\n", args[2]);
					return 1;
				}
			}
		}
		else {
			max_depth = atoi(args[1]);
			if (max_depth <= 0) {
				fprintf(stderr, "shield: invalid depth value %s\n", args[2]);
				return 1;
			}
		}
	}
	return list_directory(path, 0, max_depth);
}

int shield_cd(char** args) {
	if (args[1] == NULL) {
		fprintf(stderr, "shield: expected argument to \"cd\"\n");
	} 
	else if (chdir(args[1]) != 0) {
		perror("shield: cd failed");
		return 1;
	}
	return 0;
}

int shield_pwd(char** args) {
	char* buf;
	char* cwd;
	buf = malloc(1024 * sizeof(char*));
	if ((cwd = getcwd(buf, 1024)) != NULL) {
		printf("Current working directory: %s\n", cwd);
		free(buf);
		return 0;
	}
	else {
		perror("shield: getcwd call failed\n");
		return 1;
	}
}

int shield_exit(char** args) {
	for (int i = 0; i < aliases_length; i++) {
		free(aliases[i].name);
		for (int j = 0; aliases[i].command[j]; j++) {
			free(aliases[i].command[j]);
		}
		free(aliases[i].command);
	}
	free(aliases);
	return -1;
}

int shield_find(char** args) {
	char* name = NULL;
	int max_depth = 0;
	bool single = true;
	if (args[1] == NULL) {
		fprintf(stderr, "shield: name needed for find command\n");
		return 1;
	}
	name = args[1];
	if (args[2] != NULL) {
		if (!strcmp("all", args[2]))
			single = false;
		else if (!strcmp("single", args[2]))
			single = true;
		else {
			fprintf(stderr, "shield: invalid number of returns ('all' or 'single'), you gave: %s\n", args[2]);
			return 1;
		}
		if (args[3] != NULL) {
			max_depth = atoi(args[3]);
			if (max_depth < 0) {
				fprintf(stderr, "shield: invalid depth value %s\n", args[2]);
				return 1;
			}
		}
	}
	find(getenv("HOME"), name, 0, max_depth, single);
	return 0;
}

int shield_help(char** args) {
	if (args[1] == NULL) {
		printf("Available commands:\n");
		for (int i = 0; i < num_builtins(); i++) {
			printf("\t%s - %s\n", builtins[i].name, builtins[i].desc);
		}
		return 0;
	}
	for (int i = 0; i < num_builtins(); i++) {
		if (!strcmp(builtins[i].name, args[1])) {
			printf("Help for %s:\n%s\n", builtins[i].name, detailed_help[i]);
			return 0;
		}
	}
	fprintf(stderr, "%s command is not supported yet", args[1]);
	return 1;
}

int shield_clear(char** args) {
	printf("\e[1;1H\e[2J");
	return 0;
}

int shield_out(char** args) {
	if (args[1] == NULL) {
		printf("\n");
		return 0;
	}
	for (int i = 1; args[i]; i++) {
		printf("%s ", args[1]);
	}
	printf("\n");
	return 0;
}

int shield_time(char** args) {
	if (args[1] == NULL) {
		fprintf(stderr, "shield: expected argument for \"time\".\n");
		return 1;
	}
	clock_t before = clock();
	char** shifted_args = args + 1;
	execute_command(shifted_args);
	clock_t after = clock();
	double time_spent = (double)(after - before) / CLOCKS_PER_SEC;
	printf("%s took %f seconds to execute.\n", args[1], time_spent);
	return 0;
}

int shield_history(char** args) {
	HIST_ENTRY** hist_list = history_list();
	if (hist_list) {
		for (int i = 0; hist_list[i]; i++) {
			printf("%d: %s\n", i + 1, hist_list[i]->line);
		}
	}
	return 0;
}

int shield_get_content(char** args) {
	if (args[1] == NULL) {
		fprintf(stderr, "shield: expected argument for \"get-content\".\n");
		return 1;
	}
	FILE* fptr = fopen(args[1], "r");
	if (fptr == NULL) {
		fprintf(stderr, "shield: couldn't open %s.\n", args[1]);
		return 1;
	}
	struct stat sb;
	if (stat(args[1], &sb) == -1) {
		perror("shield: stat error.\n");
		exit(1);
	}
	if (sb.st_size == 0) {
		printf("\n");
		return 0;
	}
	else {
		char* buf = malloc((sb.st_size + 1) * sizeof(char*));
		if (buf == NULL) {
			perror("shield: malloc failed\n");
			return 1;
		}
		if ((fread(buf, sizeof(int), sb.st_size, fptr)) == -1) {
			perror("shield: read failed\n");
			return 1;
		}
		buf[sb.st_size] = '\0';
		if ((write(STDOUT_FILENO, buf, sb.st_size)) == -1) {
			perror("shield: write failed\n");
			return 1;
		}
	}
	fclose(fptr);
	return 0;
}

void update_env_file() {
	FILE* fp = fopen("shield.env", "w");
	for (int i = 0; i < env_length; i++) {
		fprintf(fp, "%s=%s\n", env[i].name, env[i].value);
	}
	fclose(fp);
}

void update_alias_file() {
	FILE* fp = fopen("shield.aliases", "w");
	for (int i = 0; i < aliases_length; i++) {
		fprintf(fp, "%s=", aliases[i].name);
		fprintf(fp, "%s", aliases[i].command[0]);
		for (int j = 1; aliases[i].command[j]; j++) {
			fprintf(fp, " %s", aliases[i].command[j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

int add_alias(char** args) {
	if (aliases_length == 0 || aliases == NULL) {
		aliases = malloc(sizeof(Alias) * (aliases_length + 1));
		if (!aliases) {
			perror("shield: malloc failed\n");
			return 1;
		}
	}
	int idx_to_set = -1;
	bool override = false;
	for (int i = 0; i < aliases_length; i++) {
		if (!strcmp(aliases[i].name,args[1])) {
			printf("shield: %s is already assigned as an alias.\n");
			char* c = readline("Would you like to update the alias? (press y/n): ");
			switch (c[0]) {
				case 'y':
					idx_to_set = i;
					override = true;
					break;
				case 'n':
					return 1;
				default:
					fprintf(stderr, "shield: not 'y' or 'n' character, aborting reassign...\n");
					return 1;
			}
		}
	}
	char* name = args[1];
	int command_len = 0;
	while (args[2 + command_len]) command_len++;
	char** alias_command = malloc((command_len + 1) * sizeof(char*));
	for (int i = 0; i < command_len; i++) {
		alias_command[i] = strdup(args[2 + i]);
	}
	alias_command[command_len] = NULL;
	aliases = realloc(aliases, sizeof(Alias) * (aliases_length + 1));
	if (!aliases) {
		perror("shield: realloc failed\n");
		return 1;
	}
	if (idx_to_set == -1)
		idx_to_set = aliases_length;
	aliases[idx_to_set].name = strdup(name);
	aliases[idx_to_set].command = alias_command;
	printf("new alias added to %s:", name);
	for (int i = 0; i < command_len; i++) {
		printf(" %s", args[2 + i]);
	}
	printf("\n");
	if (!override)
		aliases_length++;
	update_alias_file();
	return 0;
}

int shield_alias(char** args) {
	if (args[1] == NULL) {
		if (aliases_length > 0) {
			for (int i = 0; i < aliases_length; i++) {
				printf("alias %s :", aliases[i].name);
				for (int j = 0; aliases[i].command[j]; j++) {
					printf(" %s", aliases[i].command[j]);
				}
				printf("\n");
			}
		}
		else {
			printf("You don't have any aliases yet.\n");
		}
		return 0;
	}
	if (args[2] == NULL) {
		fprintf(stderr, "shield: expected argument for \"alias\".\n");
		return 1;
	}
	return add_alias(args);
}

int shield_env(char** args) {
	if (env_length > 0) {
		printf("shield environment variables:\n");
		for (int i = 0; i < env_length; i++) {
			printf("%s=%s\n", env[i].name, env[i].value);
		}
		return 0;
	}
	printf("You don't have any environment variables yet.\n");
	return 0;
}

int add_env(char** args) {
	if (env_length == 0 || env == NULL) {
		env = malloc(sizeof(EnvVar) * (env_length + 1));
		if (!env) {
			perror("shield: malloc failed\n");
			return 1;
		}
	}
	int idx_to_set = -1;
	bool override = false;
	for (int i = 0; i < env_length; i++) {
		if (!strcmp(env[i].name,args[1])) {
			printf("shield: %s is already assigned as an environment variable.\n");
			char* c = readline("Would you like to update the environment variable? (press y/n): ");
			switch (c[0]) {
				case 'y':
					idx_to_set = i;
					override = true;
					break;
				case 'n':
					return 1;
				default:
					fprintf(stderr, "shield: not 'y' or 'n' character, aborting reassign...\n");
					return 1;
			}
		}
	}
	char* name = args[1];
	char* value = args[2];
	env = realloc(env, sizeof(EnvVar) * (env_length + 1));
	if (!env) {
		perror("shield: realloc failed\n");
		return 1;
	}
	if (idx_to_set == -1)
		idx_to_set = env_length;
	env[idx_to_set].name = strdup(name);
	env[idx_to_set].value = strdup(value);
	printf("shield: new environment variable added to %s:%s\n", name, value);
	if (!override)
		env_length++;
	update_env_file();
	return 0;
}

int shield_setenv(char** args) {
	if (args[1] == NULL || args[2] == NULL) {
		fprintf(stderr, "shield: expected argument for \"setenv\".\n");
		return 1;
	}
	return add_env(args);
}

int shield_unsetenv(char** args) {
	if (args[1] == NULL) {
		fprintf(stderr, "shield: unsetenv requires a variable name\n");
		return 1;
	}

	for (int i = 0; i < env_length; i++) {
		if (strcmp(env[i].name, args[1]) == 0) {
			free(env[i].name);
			free(env[i].value);
			for (int j = i; j < env_length - 1; j++) {
				env[j] = env[j + 1];
			}
			env_length--;
			update_env_file();  // update saved file
			printf("shield: %s has been unset\n", args[1]);
			return 0;
		}
	}

	fprintf(stderr, "shield: environment variable '%s' not found\n", args[1]);
	return 1;
}

int num_builtins() {
	return sizeof(builtins) / sizeof(BuiltInCommand);
}
