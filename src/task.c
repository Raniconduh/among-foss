#include "task.h"
#include "server.h"


struct task tasks[] = {
	/* ID                  Description    Location */
	{  TASK_CAFE_TRASH,  "Empty trash", &locations[LOC_STORAGE] },
	{  TASK_CAFE_COFFEE, "Start the coffee maker", &locations[LOC_CAFETERIA] }
};


/* Assign random tasks to the specified player. */
void assign_tasks(int pid) {
	if (players[pid].fd == -1)
			return;

	int temp = random_num(TASK_COUNT);

	/* Assign random tasks. */
	for (int i = 0; i < NUM_TASKS; i++) {
retry:
		temp = random_num(TASK_COUNT);

		for(int j = 0; j < NUM_TASKS; j++) {
			if(players[pid].tasks[j] != NULL && players[pid].tasks[j]->id == temp)
				goto retry;
		}

		players[pid].tasks[i] = &tasks[temp];
		players[pid].tasks_done[i] = 0;
	}
}
