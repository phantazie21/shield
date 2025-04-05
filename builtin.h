typedef int (*builtin_func)(char** args);

typedef struct {
	char* name;
	builtin_func function;
	char* desc;
} BuiltInCommand;

typedef struct {
	char* name;
	char** command;
} Alias;

typedef struct {
	char* name;
	char* value;
} EnvVar;

int shield_cd(char** args);
int shield_exit(char** args);
int shield_list(char** args);
int shield_pwd(char** args);
int shield_find(char** args);
int shield_help(char** args);
int shield_clear(char** args);
int shield_out(char** args);
int shield_time(char** args);
int shield_history(char** args);
int shield_get_content(char** args);
int shield_alias(char** args);
int shield_env(char** args);
int shield_setenv(char** args);
int shield_unsetenv(char** args);

extern EnvVar* env;
extern int env_length;
extern Alias* aliases;
extern int aliases_length;
extern BuiltInCommand builtins[];

int num_builtins();
