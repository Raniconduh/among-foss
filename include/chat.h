#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "game.h"
#include "util.h"


char *sanitize(char *input);

int is_valid_name(char *name, int fd);

void parse_command(int pid, char *command, char *arguments[]);
