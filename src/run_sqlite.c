#include "run_sqlite.h"
#include <sqlite3.h>
#include <stdio.h>

void execute_sqlite_query(const char* dbfile, const char* query) {
    if (!dbfile || !query) {
        return;
    }

    sqlite3* conn = NULL;
    sqlite3_stmt* new_query = NULL;

    int rc = sqlite3_open(dbfile, &conn);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(conn));
        if (conn) {
            sqlite3_close(conn);
        }
        conn = NULL;
        return;
    }

    printf("\nExecuting query on SQLite:\n%s\n\n", query);

    rc = sqlite3_prepare_v2(conn, query, -1, &new_query, NULL);
    if ( rc!= SQLITE_OK) {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return;
    }

    int num_cols = sqlite3_column_count(new_query);
    if (num_cols <= 0) {
        fprintf(stderr, "No columns in result\n");
        sqlite3_finalize(new_query);
        sqlite3_close(conn);
        return;
    }

    // Print header row
    for (int i = 0; i < num_cols; ++i) {
        const char* col = sqlite3_column_name(new_query, i);
        printf("%s\t", col ? col : "");
    }
    if (num_cols > 0) {
        printf("\n----------------------------------------------------------------\n");
    }

    int execution_result;
    execution_result = sqlite3_step(new_query);
    if (execution_result == SQLITE_DONE) {
        printf("No data\n");
        sqlite3_finalize(new_query);
        sqlite3_close(conn);
        return;
    }

    if (execution_result != SQLITE_ROW) {
        fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(conn));
        sqlite3_finalize(new_query);
        sqlite3_close(conn);
        return;
    }

    while (execution_result== SQLITE_ROW) {
        for (int i = 0; i < num_cols; ++i) {
            const unsigned char* column_text = sqlite3_column_text(new_query, i);
            printf("%s\t", column_text ? (const char*)column_text : "NULL");
        }
        printf("\n");
        execution_result = sqlite3_step(new_query);
    }

    if (execution_result != SQLITE_DONE) {
        fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(conn));
    }

    sqlite3_finalize(new_query);
    sqlite3_close(conn);
}
