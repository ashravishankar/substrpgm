#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "config.h" 
#include "query_builder.h" 
#include "run_sqlite.h"

#define DEFAULT_CONFIG_FILE "config/config.json"

static void show_usage_help(const char* pgm) {
    printf("\nUsage:\n");
    printf("  %s [options]\n\n", pgm);
    printf("Options:\n");
    printf("  --database <DB_NAME>     Select target database (e.g.PostgreSQL, sqlite)\n");
    printf("  --query \"<query>\"        Provide the SQL query to convert\n");
    printf("  --config <path>          Use a custom JSON configuration file (default: config/config.json)\n");
    printf("  --list-databases         List supported database engines and exit\n");
    printf("  --export <file>          Write converted query to file instead of stdout\n");
    printf("  --execute <db_file>      Build and execute query on specified SQLite DB (requires --database sqlite)\n");
    printf("  --help                   Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --database PostgreSQL --query \"SELECT STRING_SLICE(name,1,3) FROM users;\"\n", pgm);
    printf("  %s --database sqlite --query \"SELECT STRING_SLICE(name,1,3) FROM users;\" --execute test.db\n\n", pgm);
}

static void list_databases(const Mapping_table* db_table) {

    if (!db_table) {
        fprintf(stderr, "Invalid database table\n");
        return;
    }

    //collect unique database names
    char found_db[256][64];
    int count = 0;
    memset(found_db, 0, sizeof(found_db));
    
    if (!db_table->cmd_map) {
        fprintf(stderr, "No database functions defined\n");
        return;
    }

    printf("Supported database engines (from configuration):\n");
    for (int i = 0; i < db_table->mapping_count; ++i) {

        Cmd_Mapping* map = &db_table->cmd_map[i];
        if (!map) {
            continue;
        }

        for (int db_index = 0; db_index < map->database_count; ++db_index) {

            const char* database = map->database[db_index];
            if (strlen(database) == 0) {
                continue;
            }

            //check if already listed
            bool db_done = false;

            for (int check_index = 0; check_index < count; ++check_index) {
                if (strcasecmp(found_db[check_index], database) == 0) {
                    db_done = true;
                    break;
                }
            }
            if (!db_done) {
                strncpy(found_db[count++], database, sizeof(found_db[0]) - 1);
                printf("  - %s\n", database);
            }
        }
    }
}

int main(int argc, char* argv[]) {

    const char* database = NULL;
    const char* query = NULL;
    const char* config_file_path = DEFAULT_CONFIG_FILE;
    const char* output_file = NULL;
    const char* sqlite_database_file = NULL;
    bool db_only = false;

    if (argc < 2) {
        show_usage_help(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (i + 1 < argc) {
            if (strcmp(argv[i], "--database") == 0) {
                database = argv[++i];
            } else if (strcmp(argv[i], "--query") == 0) {
                query = argv[++i];
            } else if (strcmp(argv[i], "--config") == 0) {
                config_file_path = argv[++i];
            } else if (strcmp(argv[i], "--export") == 0) {
                output_file = argv[++i];
            } else if (strcmp(argv[i], "--execute") == 0) {
                sqlite_database_file = argv[++i];
            }

        } else if (strcmp(argv[i], "--list-databases") == 0) {
            db_only = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            show_usage_help(argv[0]);
            return 0;
        }
    }

    Mapping_table db_table = {0};
    int rc = load_db_funcs(config_file_path, &db_table);
    if (!rc) {
        fprintf(stderr, "Failed to load configuration from %s\n", config_file_path);
        return 1;
    }

    if (db_only) {
        list_databases(&db_table);
        cleanup_db_table(&db_table);
        return 0;
    }

    if (!database || !query) {
        fprintf(stderr, "Error: --database and --query are required.\n");
        show_usage_help(argv[0]);
        cleanup_db_table(&db_table);
        return 1;
    }

    char* result = convert_db_query(query, database, &db_table);
    if (!result) {
        fprintf(stderr, "Error: cannot generate query for database '%s'\n", database);
        cleanup_db_table(&db_table);
        return 1;
    }

    if (output_file) {
        FILE* output_handle = fopen(output_file, "w");
        if (!output_handle) {
            fprintf(stderr, "Unable to open export file");
        } else {
            fprintf(output_handle, "%s\n", result);
            fclose(output_handle);
            printf("Exported converted query to %s\n", output_file);
        }
    } else {
        printf("\n[%s] Converted Query:\n%s\n", database, result);
    }

    if (sqlite_database_file) {
        if (strcasecmp(database, "sqlite") != 0) {
            fprintf(stderr, "Error: --execute is implemented for sqlite only (database must be 'sqlite')\n");
        } else {
            execute_sqlite_query(sqlite_database_file, result);
        }
    }

    free(result);
    cleanup_db_table(&db_table);
    return 0;
}
