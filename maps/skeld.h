/*
 * Header file for the Skeld Map of  Among-Sus
 */

const char skeld_map[] =
	"|\\----------------|--------------|----------------|--------------\\\n"
	"|                                                                 \\\n"
	"| UPPER ENGINE                        CAFETERIA       WEAPONS      \\\n"
	"|                 |-     --------|                |                 \\\n"
	"|/--------|    |--|       MEDBAY |                |                  \\\n"
	"          |    |                 |                |                   \\------\\\n"
	"/---------|    |-------\\         |                |----------|        |       \\\n"
	"|         |    |        \\        |---|     |------|          |                 |\n"
	"|                        \\       |                |                            |\n"
	"| REACTOR        SECURITY |      |  ADMIN OFFICE  |   O2           NAVIGATION  |\n"
	"|                         |      |                |          |                 |\n"
	"|         |    |          |      |---|     |----|-|----------|                 |\n"
	"\\---------|    |----------|------|              |                     |       /\n"
	"          |    |                 |                                    /------/\n"
	"|\\--------|    |--|              |                                   /\n"
	"|                 |              |              |--    --|          /\n"
	"| LOWER ENGINE       ELECTRICAL       STORAGE   | COMMS  | SHIELDS /\n"
	"|                                               |        |        /\n"
	"|/----------------|--------------|--------------|--------|-------/\n"
;


enum skeld_player_task_short {
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
	TASK_OS_WATER,
	TASK_MEDBAY_WIRES,
	TASK_UPPER_CATALYZER,
	TASK_LOWER_CATALYZER,
	TASK_UPPER_COMPRESSION_COIL,
	TASK_LOWER_COMPRESSION_COIL,
	TASK_SHORT_COUNT,
};

const char skeld_short_task_descriptions[][45] = {
	"Empty the cafeteria trash",
	"Start the coffee maker in the cafeteria",
	"Fix wiring in cafeteria",
	"Empty the storage trash chute",
	"Fix wiring in storage",
	"Clean the floor in storage",
	"Fix wiring in electrical",
	"Reset breakers in electrical",
	"Fix wiring in admin",
	"Clean the floor in admin",
	"Fix wiring in navigation",
	"Adjust course in navigation",
	"Check headings in navigation",
	"Fix wiring in weapons",
	"Calibrate targeting system in weapons",
	"Fix wiring in shields",
	"Fix wiring in o2",
	"Clean oxygenator filter in o2",
	"Water plants in o2",
	"Fix wiring in medbay",
	"Check catalyzer in upper engine",
	"Check catalyzer in lower engine",
	"Replace compression coil in upper engine",
	"Replace compression coil in lower engine",
};

enum skeld_player_task_long {
	TASK_SHIELDS_POWER,
	TASK_WEAPONS_POWER,
	TASK_NAV_LOG,
	TASK_SHIELD_LOG,
	TASK_REACTOR_FUEL,
	TASK_POTATO,
	TASK_LONG_COUNT
};

const char skeld_long_task_descriptions[][2][45] = {
	{"Route power to defence in electrical", "Accept rerouted power in shields"},
	{"Route power to attack in electrical", "Accept rerouted power in weapons"},
	{"Download the latest navigation data", "Upload data in admin"},
	{"Download the latest shields data", "Upload data in admin"},
	{"Pick up nuclear fuel in storage", "Insert fuel into reactor"},
	{"Pick up potato in cafeteria", "Plant the potato in o2"},
	{"Get radio log from communications", "Deliver communications log to admin"},
};

enum skeld_player_location {
	LOC_CAFETERIA,
	LOC_REACTOR,
	LOC_UPPER_ENGINE,
	LOC_LOWER_ENGINE,
	LOC_SECURITY,
	LOC_MEDBAY,
	LOC_ELECTRICAL,
	LOC_STORAGE,
	LOC_ADMIN,
	LOC_COMMUNICATIONS,
	LOC_O2,
	LOC_WEAPONS,
	LOC_SHIELDS,
	LOC_NAVIGATION,
	LOC_COUNT,
};

const char skeld_locations[][45] = {
	[LOC_CAFETERIA] = "cafeteria",
	[LOC_REACTOR] = "reactor",
	[LOC_UPPER_ENGINE] = "upper",
	[LOC_LOWER_ENGINE] = "lower",
	[LOC_SECURITY] = "security",
	[LOC_MEDBAY] = "medbay",
	[LOC_ELECTRICAL] = "electrical",
	[LOC_STORAGE] = "storage",
	[LOC_ADMIN] = "admin",
	[LOC_COMMUNICATIONS] = "communications",
	[LOC_O2] = "o2",
	[LOC_WEAPONS] = "weapons",
	[LOC_SHIELDS] = "shields",
	[LOC_NAVIGATION] = "navigation",
};

enum skeld_player_location doors[][10] = {
	[LOC_CAFETERIA]     = { 
                          LOC_MEDBAY, 
                          LOC_ADMIN,
                          LOC_WEAPONS,
                          LOC_COUNT 
                        },
	[LOC_REACTOR]       = { 
                          LOC_UPPER_ENGINE, 
                          LOC_SECURITY, 
                          LOC_LOWER_ENGINE, 
                          LOC_COUNT 
                        },
	[LOC_UPPER_ENGINE]  = { 
                          LOC_REACTOR, 
                          LOC_SECURITY, 
                          LOC_MEDBAY, 
                          LOC_COUNT 
                        },
	[LOC_LOWER_ENGINE]  = { 
                          LOC_REACTOR, 
                          LOC_SECURITY, 
                          LOC_ELECTRICAL, 
                          LOC_COUNT 
                         },
	[LOC_SECURITY]       = { 
                          LOC_UPPER_ENGINE,
                          LOC_REACTOR, 
                          LOC_LOWER_ENGINE, 
                          LOC_COUNT 
                         },
	[LOC_MEDBAY]         = { 
                          LOC_UPPER_ENGINE, 
                          LOC_CAFETERIA, 
                          LOC_COUNT 
                         },
	[LOC_ELECTRICAL]     = { 
                          LOC_LOWER_ENGINE, 
                          LOC_STORAGE,
                          LOC_COUNT 
                         },
	[LOC_STORAGE]        = { 
                          LOC_ELECTRICAL, 
                          LOC_ADMIN, 
                          LOC_COMMUNICATIONS, 
                          LOC_SHIELDS, 
                          LOC_COUNT 
                         },
	[LOC_ADMIN]          = { 
                          LOC_CAFETERIA,
                          LOC_STORAGE, 
                          LOC_COUNT 
                         },
	[LOC_COMMUNICATIONS] = { 
                          LOC_STORAGE, 
                          LOC_SHIELDS, 
                          LOC_COUNT
                         },
	[LOC_O2]             = { 
                          LOC_SHIELDS, 
                          LOC_WEAPONS, 
                          LOC_NAVIGATION, 
                          LOC_COUNT 
                         },
	[LOC_WEAPONS]        = { 
                          LOC_CAFETERIA, 
                          LOC_O2, 
                          LOC_NAVIGATION, 
                          LOC_COUNT
                         },
	[LOC_SHIELDS]        = { 
                          LOC_STORAGE, 
                          LOC_COMMUNICATIONS, 
                          LOC_O2, 
                          LOC_NAVIGATION, 
                          LOC_COUNT 
                         },
	[LOC_NAVIGATION]     = { 
                          LOC_WEAPONS, 
                          LOC_O2, 
                          LOC_SHIELDS, 
                          LOC_COUNT 
                         },
};

const char skeld_descriptions[][256] = {
	[LOC_CAFETERIA] = "You are standing in the middle of the cafeteria, in the center there's an emergency button\n",
	[LOC_REACTOR] = "You are in the reactor room, it seems to be running normally\n",
	[LOC_UPPER_ENGINE] = "You are in a small room, mostly filled up by an engine.\n",
	[LOC_LOWER_ENGINE] = "You are in a small room, mostly filled up by an engine.\n",
	[LOC_SECURITY] = "You are in a small room filled with monitors, the monitors are displaying camera images showing an overview of the ship\n",
	[LOC_MEDBAY] = "You are in a room with beds and a medical scanner.\n",
	[LOC_ELECTRICAL] = "You are in a room filled with equipment racks. Some of them have wires sticking out of them\n",
	[LOC_STORAGE] = "You are in a large room filled with boxes. One of the walls has a large door to the outside\n",
	[LOC_ADMIN] = "You are in a nice carpeted room with a holographic map in the middle\n",
	[LOC_COMMUNICATIONS] = "You are in a small room with what looks like radio equipment\n",
	[LOC_O2] = "You are in a room with plants in terrariums and life support equipment\n",
	[LOC_WEAPONS] = "You are in a circular room with a targeting system in the middle and a view of outer space\n",
	[LOC_SHIELDS] = "You are in a circular room with glowing tubes and a control panel for the shields\n",
	[LOC_NAVIGATION] = "You are all the way in the front of the ship in a room with the ship controls and a great view of space\n",
};
