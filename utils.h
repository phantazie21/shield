#pragma once

#include <stdio.h>
#include <stdlib.h>

char* read_line();
char** split_line(char* input);
int execute_command(char** args);
