#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/config.h"
#include "../include/query_builder.h"

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            return 0; \
        } else { \
            printf("PASS: %s\n", message); \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("\n--- Running %s ---\n", #test_func); \
        if (test_func()) { \
            printf("✓ %s PASSED\n", #test_func); \
            tests_passed++; \
        } else { \
            printf("✗ %s FAILED\n", #test_func); \
            tests_failed++; \
        } \
        total_tests++; \
    } while(0)

static int total_tests = 0;
static int tests_passed = 0;
static int tests_failed = 0;

//Create a simple test mapping table
Mapping_table* create_test_mapping_table() {
    Mapping_table* table = malloc(sizeof(Mapping_table));
    if (!table) return NULL;
    
    table->cmd_map = malloc(2 * sizeof(Cmd_Mapping));
    if (!table->cmd_map) {
        free(table);
        return NULL;
    }
    
    table->mapping_count = 2;
    
    //First mapping: CMD_SUBSTRING
    strcpy(table->cmd_map[0].command, "CMD_SUBSTRING");
    strcpy(table->cmd_map[0].database[0], "PostgreSQL");
    strcpy(table->cmd_map[0].db_funcs[0], "substring");
    strcpy(table->cmd_map[0].database[1], "sqlite");
    strcpy(table->cmd_map[0].db_funcs[1], "substr");
    strcpy(table->cmd_map[0].database[2], "MySQL");
    strcpy(table->cmd_map[0].db_funcs[2], "substr");
    table->cmd_map[0].database_count = 3;
    
    //Second mapping: CMD_LENGTH
    strcpy(table->cmd_map[1].command, "CMD_LENGTH");
    strcpy(table->cmd_map[1].database[0], "PostgreSQL");
    strcpy(table->cmd_map[1].db_funcs[0], "char_length");
    strcpy(table->cmd_map[1].database[1], "sqlite");
    strcpy(table->cmd_map[1].db_funcs[1], "length");
    strcpy(table->cmd_map[1].database[2], "MySQL");
    strcpy(table->cmd_map[1].db_funcs[2], "length");
    table->cmd_map[1].database_count = 3;
    
    return table;
}

//Cleanup after test is complete
void cleanup_test_table(Mapping_table* table) {
    if (table) {
        if (table->cmd_map) {
            free(table->cmd_map);
        }
        free(table);
    }
}

//Test 1: Test query conversion for SQLite
int test_query_sqlite() {
    Mapping_table* table = create_test_mapping_table();
    TEST_ASSERT(table != NULL, "Test mapping table created successfully");
    
    const char* input = "SELECT CMD_SUBSTRING(name, 1, 3) FROM users";

    char* result = convert_db_query(input, "sqlite", table);
    
    TEST_ASSERT(result != NULL, "Query conversion returned non-NULL result");
    TEST_ASSERT(strstr(result, "substr") != NULL, "Query contains 'substr' function");
    TEST_ASSERT(strstr(result, "CMD_SUBSTRING") == NULL, "Query no longer contains 'CMD_SUBSTRING'");
    
    printf("Input:  %s\n", input);
    printf("Output: %s\n", result);
    
    free(result);
    cleanup_test_table(table);
    return 1;
}

// Test 2: Multiple function conversion
int test_multiple_functions() {
    Mapping_table* table = create_test_mapping_table();
    TEST_ASSERT(table != NULL, "Test mapping table created successfully");
    
    const char* input = "SELECT CMD_SUBSTRING(name, 1, CMD_LENGTH(name)) FROM users";
    char* result = convert_db_query(input, "PostgreSQL", table);
    
    TEST_ASSERT(result != NULL, "Query conversion returned non-NULL result");
    TEST_ASSERT(strstr(result, "substring") != NULL, "Query contains 'substring' function");
    TEST_ASSERT(strstr(result, "char_length") != NULL, "Query contains 'char_length' function");
    TEST_ASSERT(strstr(result, "CMD_SUBSTRING") == NULL, "Query no longer contains 'CMD_SUBSTRING'");
    TEST_ASSERT(strstr(result, "CMD_LENGTH") == NULL, "Query no longer contains 'CMD_LENGTH'");
    
    printf("Input:  %s\n", input);
    printf("Output: %s\n", result);
    
    free(result);
    cleanup_test_table(table);
    return 1;
}

//Test 3: Unknown database handling
int test_unknown_database() {
    Mapping_table* table = create_test_mapping_table();
    TEST_ASSERT(table != NULL, "Test mapping table created successfully");
    
    const char* input = "SELECT CMD_SUBSTRING(name, 1, 3) FROM users";
    char* result = convert_db_query(input, "UnknownDB", table);
    
    TEST_ASSERT(result != NULL, "Query conversion returned non-NULL result");
    TEST_ASSERT(strcmp(result, input) == 0, "Query unchanged for unknown database");
    
    printf("Input:  %s\n", input);
    printf("Output: %s\n", result);
    
    free(result);
    cleanup_test_table(table);
    return 1;
}

//Test 4: NULL input handling
int test_null_inputs() {
    Mapping_table* table = create_test_mapping_table();
    TEST_ASSERT(table != NULL, "Test mapping table created successfully");
    
    char* result1 = convert_db_query(NULL, "PostgreSQL", table);
    TEST_ASSERT(result1 == NULL, "NULL query input handled correctly");
    
    char* result2 = convert_db_query("SELECT * FROM users", NULL, table);
    TEST_ASSERT(result2 == NULL, "NULL database input handled correctly");
    
    char* result3 = convert_db_query("SELECT * FROM users", "PostgreSQL", NULL);
    TEST_ASSERT(result3 == NULL, "NULL table input handled correctly");
    
    cleanup_test_table(table);
    return 1;
}

//Test 5: Configuration loading test
int test_config_loading() {
    Mapping_table table = {0};
    
    //Test loading the actual config file
    bool success = load_db_funcs("config/config.json", &table);
    TEST_ASSERT(success == true, "Configuration file loaded successfully");
    TEST_ASSERT(table.mapping_count > 0, "Configuration contains mappings");
    TEST_ASSERT(table.cmd_map != NULL, "Command mappings allocated");
    
    printf("Loaded %d command mappings from config file\n", table.mapping_count);
    
    //Test a real conversion using loaded config
    const char* test_query = "SELECT CMD_SUBSTRING(name, 1, 3) FROM users";
    char* result = convert_db_query(test_query, "PostgreSQL", &table);
    TEST_ASSERT(result != NULL, "Query conversion with loaded config works");
    
    printf("Real config test - Input:  %s\n", test_query);
    printf("Real config test - Output: %s\n", result);
    
    free(result);
    cleanup_db_table(&table);
    return 1;
}

int main() {
    printf("**** SubstrPgm Unit Tests ***\n");
    
    //Run all tests
    RUN_TEST(test_query_sqlite);
    RUN_TEST(test_multiple_functions);
    RUN_TEST(test_unknown_database);
    RUN_TEST(test_null_inputs);
    RUN_TEST(test_config_loading);
    
    //Print summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / total_tests * 100);
    
    return tests_failed == 0 ? 0 : 1;
}
