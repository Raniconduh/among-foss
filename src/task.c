#include "server.h"


struct task tasks[] = {
	/* ID                           Description                   Location */
	{  TASK_CAFE_TRASH,             "Empty trash",                &locations[LOC_CAFETERIA]     },
	{  TASK_CAFE_COFFEE,            "Start the coffee maker",     &locations[LOC_CAFETERIA]     },
	{  TASK_CAFE_WIRES,             "Fix wiring",                 &locations[LOC_CAFETERIA]     },
	{  TASK_STORAGE_TRASH,          "Empty the trash chute",      &locations[LOC_STORAGE]       },
	{  TASK_STORAGE_WIRES,          "Fix wiring",                 &locations[LOC_STORAGE]       },
	{  TASK_STORAGE_CLEAN,          "Clean the floor",            &locations[LOC_STORAGE]       },
	{  TASK_ELECTRICAL_WIRES,       "Fix wiring",                 &locations[LOC_ELECTRICAL]    },
	{  TASK_ELECTRICAL_BREAKERS,    "Reset breakers",             &locations[LOC_ELECTRICAL]    },
	{  TASK_ADMIN_WIRES,            "Fix wiring",                 &locations[LOC_ADMIN]         },
	{  TASK_ADMIN_CLEAN,            "Clean the floor",            &locations[LOC_ADMIN]         },
	{  TASK_NAVIGATION_WIRES,       "Fix wiring",                 &locations[LOC_NAVIGATION]    },
	{  TASK_NAVIGATION_COURSE,      "Adjust course",              &locations[LOC_NAVIGATION]    },
	{  TASK_NAVIGATION_HEADING,     "Check headings",             &locations[LOC_NAVIGATION]    },
	{  TASK_WEAPONS_WIRES,          "Fix wiring",                 &locations[LOC_WEAPONS]       },
	{  TASK_WEAPONS_CALIBRATE,      "Calibrate targeting system", &locations[LOC_WEAPONS]       },
	{  TASK_SHIELDS_WIRES,          "Fix wiring",                 &locations[LOC_SHIELDS]       },
	{  TASK_O2_WIRES,               "Fix wiring",                 &locations[LOC_O2]            },
	{  TASK_O2_CLEAN,               "Clean oxygenator filter",    &locations[LOC_O2]            },
	{  TASK_O2_WATER,               "Water plants",               &locations[LOC_O2]            },
	{  TASK_MEDBAY_WIRES,           "Fix wiring",                 &locations[LOC_MEDBAY]        },
	{  TASK_UPPER_CATALYZER,        "Check catalyzer",            &locations[LOC_UPPER_ENGINE]  },
	{  TASK_LOWER_CATALYZER,        "Check catalyzer",            &locations[LOC_LOWER_ENGINE]  },
	{  TASK_UPPER_COMPRESSION_COIL, "Replace compression coil",   &locations[LOC_UPPER_ENGINE]  },
	{  TASK_LOWER_COMPRESSION_COIL, "Replace compression coil",   &locations[LOC_LOWER_ENGINE]  }
};


int get_task_id(int pid, struct task *task) {
	struct player *player = &players[pid];

	for (int i = 0; i < NUM_TASKS; i++)
		if(task == player->tasks[i])
			return i;

	return -1;
}

struct task *get_task_by_description(char *description, struct location *location) {
	for (int i = 0; i < TASK_COUNT; i++) {
		struct task *task = &tasks[i];

		/* If the location or task doesn't actually exist, continue with the next object. */
		if (task->description == NULL || location->name == NULL)
			continue;

		/* If the location doesn't match, continue with the next object. */
		if (task->location != location)
			continue;

		/* If the name of the argument and found object matches, return it. */
		if (strcmp(task->description, description) == 0)
			return task;
	}

	return NULL;
}

/* Assign random tasks to the specified player. */
void assign_tasks(int pid) {
	struct player *player = &players[pid];
	int temp = random_num(TASK_COUNT);

	/* Assign random tasks. */
	for (int i = 0; i < NUM_TASKS; i++) {
retry:
		temp = random_num(TASK_COUNT);

		for(int j = 0; j < NUM_TASKS; j++) {
			if(player->tasks[j] != NULL && player->tasks[j]->id == temp)
				goto retry;
		}

		player->tasks[i] = &tasks[temp];
		player->tasks_done[i] = 0;
	}
}

int do_task(int pid, struct task *task) {
	struct player *player = &players[pid];
	struct location *location = player->location;

	/* If the task is a null pointer. */
	if (task == NULL)
		return JSON_DO_TASK_DOESNT_EXIST;

	/* If the location doesn't match. */
	if (location != task->location)
		return JSON_DO_TASK_WRONG_LOCATION;

	int task_id = get_task_id(pid, task);

	/* If the client tries to do a task it has not been assigned. */
	if (task_id == -1)
		return JSON_DO_TASK_DOESNT_EXIST;

	/* If the player already completed the task. */
	if (player->tasks_done[task_id])
		return JSON_DO_TASK_ALREADY_COMPLETED;

	player->tasks_done[task_id] = 1;
	check_win_condition();

	return JSON_DO_TASK_SUCCESS;
}
