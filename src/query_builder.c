#include "query_builder.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//Replace all occurrences of 'pattern' in 'src' with 'str'.
static char* str_query(const char* src, const char* pattern, const char* str) {

    if (!src || !pattern || !str) {
        fprintf(stderr, "Invalid arguments to str_query\n");
        return NULL;
    }

    int src_len = strlen(src);
    int pattern_len = strlen(pattern);
    int str_len = strlen(str);

    // count how many times the search text appears
    int count = 0;
    const char* temp = src;

    while ((temp = strstr(temp, pattern)) != NULL) {
        count++;
        temp += pattern_len;
    }

    if (count == 0) {
        return strdup(src);
    }

    int diff = str_len - pattern_len;
    int size = src_len + diff * count + 1;
    if (size <= 0) {
        return strdup(src);
    }

    char* result = malloc(size);
    if (!result) {
        fprintf(stderr, "Failed to allocate memory in str_query\n");
        return NULL;
    }

    const char* source = src;
    char* dest = result;
    const char* found;

    while ((found = strstr(source, pattern)) != NULL) {
        if (found == source) {
            memcpy(dest, str, str_len);
            dest += str_len;
            source += pattern_len;
            continue;
        }
        int prefix_length = found - source;

        memcpy(dest, source, prefix_length);
        dest += prefix_length;

        memcpy(dest, str, str_len);
        dest += str_len;
        source = found + pattern_len;
    }

    if (source < src + src_len) {
        int left = src + src_len - source;
        memcpy(dest, source, left);
        dest += left;
    }

    *dest = '\0';  // Add null terminator
    return result;
}

char* convert_db_query(const char* query, const char* db, const Mapping_table* db_table) {
    if (!query || !db || !db_table) {
        fprintf(stderr, "Invalid arguments to convert_query\n");
        return NULL;
    }
    
    char* dup_query = strdup(query);
    if (!dup_query) {
        fprintf(stderr, "Failed to duplicate query\n");
        return NULL;
    }

    for (int i = 0; i < db_table->mapping_count; ++i) {
        const Cmd_Mapping* map = &db_table->cmd_map[i];
        const char* func = NULL;
        
        for (int db_index = 0; db_index < map->database_count; ++db_index) {
            if (strlen(map->database[db_index]) == 0) {
                continue;
            }
            if (strlen(map->db_funcs[db_index]) == 0) {
                continue;
            }
            
            if (strcasecmp(map->database[db_index], db) == 0) {
                func = map->db_funcs[db_index];

                break;
            }
        }

        if (!func) {
            continue; //no mapping for this database; skip
        }
        
        const char* cmd = map->command;

        //str all occurrences of command with db function
        char* new_query = str_query(dup_query, cmd, func);
        free(dup_query);
        
        if (!new_query) { 
            return NULL;
        }

        dup_query = new_query;
    }


    return dup_query;
}
