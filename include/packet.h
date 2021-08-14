#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "constant.h"
#include "chat.h"
#include "game.h"
#include "task.h"
#include "json.h"


int handle_packet(int pid, char *type, struct json_object *object);
