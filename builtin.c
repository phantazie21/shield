#include "builtin.h"
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

BuiltInCommand builtins[] = {
	{"cd", shield_cd},
	{"quit", shield_exit},
	{"list", shield_list},
	{"pwd", shield_pwd},
	{"find", shield_find}
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

int num_builtins() {
	return sizeof(builtins) / sizeof(BuiltInCommand);
}
