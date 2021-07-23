/*
POLUS
*/

const char map[] =
"               REACTOR  /----------\\\n"
"                   \\   |          |\n"
"                    < > | DROPSHIP |\n"
"                    | | |          |\\\n"
"|-----------------|-| |-/          \\|------------|-------------------|\n"
"|                 |                 |            |                   |\n"
"|    ELECTRICAL       UPPER             STORAGE      LABORATORY      |\n"
"|                 |   COURTYARD     |            |                   |\n"
"|-|    |-|        |                 |------------|-------------||   ||\n"
"|SECURITY|    |---/   |-----|                                  |DECOM|\n"
"|--------|    |       |COMMS|             RIGHT COURTYARD      ||   ||\n"
"|        |    |       ||   ||                                  |     |\n"
"|        |    |                    |--------|-----|            |     |\n"
"|   O2        |    LOWER COURTYARD | OFFICE |COM-              |     |\n"
"|        |`                        || |---| |PUTER|       /----|     |\n"
"|        |    |       |--|   ||    |         ROOM |       |          |\n"
"|--------|----|       |WEAPONS|      ADMIN  |-----|-------| SPECIMEN |\n"
"              |       |       |    |         DECOM          ROOM     |\n"
"              |-------|-------|----|--------|-----|------------------|\n"

enum struct player_location_polus doors[][] = {
        [LOC_DROPSHIP]     = { LOC_UPPER_CYARD, LOC_COUNT },
        [LOC_UPPER_CYARD]  = { 
                               LOC_DROPSHIP
                               LOC_LOWER_CYARD,
                               LOC_RIGHT_CYARD,
                               LOC_ELECTRICAL,
                               LOC_STORAGE,
                               LOC_REACTOR,
                               LOC_COUNT 
                             },
        [LOC_LOWER_CYARD]  = {
                               LOC_UPPER_CYARD,
                               LOC_RIGHT_CYARD,
                               LOC_O2,
                               LOC_WEAPONS,
                               LOC_COMMS,
                               LOC_ADMIN,
                               LOC_COUNT
                             },
        [LOC_RIGHT_CYARD]  = {
                               LOC_UPPER_CYARD,
                               LOC_LOWER_CYARD,
                               LOC_COMPUTER,
                               LOC_COUNTER
                             },
        [LOC_ELECTRICAL]   = {
                               LOC_UPPER_CYARD,
                               LOC_O2,
                               LOC_SECURITY,
                               LOC_COUNT
                             },
        [LOC_SECURITY]     = { LOC_ELECTRICAL, LOC_COUNT },
        [LOC_O2]           = {
                               LOC_ELECTRICAL,
                               LOC_LOWER_CYARD,
                               LOC_COUNT
                             },
        [LOC_STORAGE]      = { 
                               LOC_UPPER_CYARD, 
                               LOC_LAB,
                               LOC_COUNT 
                             },
        [LOC_REACTOR]      = { LOC_UPPER_CYARD, LOC_COUNT },
        [LOC_LAB]          = { 
                               LOC_STORAGE,
                               LOC_UPPER_DECOM, 
                               LOC_COUNT 
                            },
        [LOC_UPPER_DECOM] = {
                               LOC_LAB,
                               LOC_SPECIMEN,
                               LOC_COUNT
                            },
        [LOC_SPECIMEN]    = {
                               LOC_UPPER_DECOM,
                               LOC_LOWER_DECOM,
                               LOC_COUNT 
                            },
        [LOC_LOWER_DECOM] = {
                               LOC_SPECIMEN,
                               LOC_ADMIN,
                               LOC_COUNT
                            },
        [LOC_ADMIN]       = {
                               LOC_LOWER_DECOM,
                               LOC_COMPUTER,
                               LOC_OFFICE,
                               LOC_LOWER_CYARD,
                               LOC_COUNT
                            },
        [LOC_OFFICE]      = { LOC_ADMIN, LOC_COUNT},
        [LOC_COMPUTER]    = { 
                              LOC_RIGHT_CYARD, 
                              LOC_ADMIN, 
                              LOC_COUNT
                            },
        [LOC_WEAPONS]     = { LOC_LOWER_CYARD, LOC_COUNT },
        [LOC_COMMS]       = { LOC_LOWER_CYARD, LOC_COUNT },
};
