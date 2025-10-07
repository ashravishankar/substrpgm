#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define MAX_DATABASES 10

//struct for database commands mappings
typedef struct {
    char command[32];              // e.g. "CMD_SUBSTRING"
    char database[MAX_DATABASES][32];    // e.g. "PostgreSQL", "MySQL", "sqlite"
    char db_funcs[MAX_DATABASES][32]; // e.g. "substr", "substring"
    int database_count;              // number of databases in the database array
} Cmd_Mapping;

typedef struct {
    Cmd_Mapping* cmd_map;
    int mapping_count;
} Mapping_table;

//Load from JSON config file. Returns true on success.
bool load_db_funcs(const char* config_file, Mapping_table* db_table);

//Free memory used by function table
void cleanup_db_table(Mapping_table* db_table);

#endif
