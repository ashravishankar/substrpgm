#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

bool load_db_funcs(const char* config_file, Mapping_table* db_table) {
    if (!config_file || !db_table) {
        fprintf(stderr, "Invalid arguments to load_db_funcs\n");
        return false;
    }



    FILE* fd = fopen(config_file, "r");
    if (!fd) {
        fprintf(stderr, "Failed to open config file %s\n", config_file);
        return false;
    }

    fseek(fd, 0, SEEK_END);
    long config_size = ftell(fd);
    rewind(fd);

    char* db_config = malloc(config_size + 1);
    if (!db_config) {
        fclose(fd);
        fprintf(stderr, "Could not allocate db_config\n");
        return false;
    }
    if (fread(db_config, 1, config_size, fd) != (size_t)config_size) {
        fclose(fd);
        free(db_config);
        fprintf(stderr, "Failed to read db_config\n");
        return false;
    }

    db_config[config_size] = '\0';
    fclose(fd);

    cJSON* json_root = cJSON_Parse(db_config);
    free(db_config);
    if (!json_root) {
        fprintf(stderr, "Failed to parse JSON db_config\n");
        return false;
    }

    int count = cJSON_GetArraySize(json_root); // number of keys (children)
    if (count <= 0) {
        cJSON_Delete(json_root);
        db_table->cmd_map = NULL;
        db_table->mapping_count = 0;
        return true;
    }

    db_table->cmd_map = (Cmd_Mapping*)calloc(count, sizeof(Cmd_Mapping));
    if (!db_table->cmd_map) {
        cJSON_Delete(json_root);
        fprintf(stderr, "Could not allocate db_table\n");
        return false;
    }

    int i = 0;
    cJSON* head = NULL;
    cJSON_ArrayForEach(head, json_root) {
        if (!head->string || !cJSON_IsObject(head)) {
            fprintf(stderr, "Invalid JSON structure\n");
            continue;
        }
        if (strlen(head->string) >= sizeof(db_table->cmd_map[i].command)) {
            fprintf(stderr, "Command name too long\n");
            continue;
        }
  
        if (cJSON_GetArraySize(head) <= 0) {
            fprintf(stderr, "No database functions defined for command\n");
            continue;
        }
        if (cJSON_GetArraySize(head) > MAX_DATABASES) {
            fprintf(stderr, "Too many database functions defined for command\n");
            continue;
        }
      
        strncpy(db_table->cmd_map[i].command,
            head->string,
                sizeof(db_table->cmd_map[i].command)-1);


        cJSON* cur = head->child;
        int j = 0;
        while (cur && j < MAX_DATABASES) {
            if (!cur->string || !cJSON_IsString(cur)) {
                fprintf(stderr, "Invalid JSON structure\n");
                continue;
            }

            if (strlen(cur->string) >= 
            sizeof(db_table->cmd_map[i].database[j])) {
                fprintf(stderr, "Database too long\n");
                continue;
            }
            if (strlen(cJSON_GetStringValue(cur)) >= 
                    sizeof(db_table->cmd_map[i].db_funcs[j])) {
                fprintf(stderr, "Function too long\n");
                continue;
            }
         

            if (cur->string && cJSON_IsString(cur)) {
                strncpy(db_table->cmd_map[i].database[j],
                        cur->string,
                        sizeof(db_table->cmd_map[i].database[j]) - 1);

                const char* func = cJSON_GetStringValue(cur);
                if (func) {
                    strncpy(db_table->cmd_map[i].db_funcs[j],
                            func,
                            sizeof(db_table->cmd_map[i].db_funcs[j]) - 1);
                }
                j++;
            }
            cur = cur->next;
        }
        db_table->cmd_map[i].database_count = j;
        i++;
    }

    db_table->mapping_count = i;
    cJSON_Delete(json_root);
    return true;
}

void cleanup_db_table(Mapping_table* db_table) {
    if (!db_table) {
        return;
    }
   
    if (db_table->cmd_map) {
        free(db_table->cmd_map);
        db_table->cmd_map = NULL;
    }
    db_table->mapping_count = 0;
}
