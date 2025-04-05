#pragma once

#include <stdio.h>
#include <stdlib.h>

void set_color_red();
void reset_color();
char* read_line();
char** split_line(char* input);
int execute_command(char** args);
int load_aliases();
int load_env();
