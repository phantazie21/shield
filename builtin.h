typedef int (*builtin_func)(char** args);

typedef struct {
	char* name;
	builtin_func function;
} BuiltInCommand;

int shield_cd(char** args);
int shield_exit(char** args);
int shield_list(char** args);
int shield_pwd(char** args);
int shield_find(char** args);

extern BuiltInCommand builtins[];

int num_builtins();
