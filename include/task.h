#pragma once

#include <stdio.h>
#include <string.h>

#include "location.h"


#define NUM_TASKS 5 /* Amount of tasks a player gets */


enum task_id {
	TASK_CAFE_TRASH,
	TASK_CAFE_COFFEE,
	TASK_CAFE_WIRES,
	TASK_STORAGE_TRASH,
	TASK_STORAGE_WIRES,
	TASK_STORAGE_CLEAN,
	TASK_ELECTRICAL_WIRES,
	TASK_ELECTRICAL_BREAKERS,
	TASK_ADMIN_WIRES,
	TASK_ADMIN_CLEAN,
	TASK_NAVIGATION_WIRES,
	TASK_NAVIGATION_COURSE,
	TASK_NAVIGATION_HEADING,
	TASK_WEAPONS_WIRES,
	TASK_WEAPONS_CALIBRATE,
	TASK_SHIELDS_WIRES,
	TASK_O2_WIRES,
	TASK_O2_CLEAN,
	TASK_O2_WATER,
	TASK_MEDBAY_WIRES,
	TASK_UPPER_CATALYZER,
	TASK_LOWER_CATALYZER,
	TASK_UPPER_COMPRESSION_COIL,
	TASK_LOWER_COMPRESSION_COIL,
	TASK_COUNT
};

struct task {
	enum task_id id;
	char *description;
	struct location *location;
};


extern struct task tasks[];


int get_task_id(int pid, struct task *task);

struct task *get_task_by_description(char *description, struct location *location);

void assign_tasks(int pid);

int do_task(int pid, struct task *task);
