#ifndef QUERY_BUILDER_H
#define QUERY_BUILDER_H

#include "config.h"
#define QUERY_BUILDER_VERSION "1.0.0"

//Convert a given SQL query to db specific syntax given in JSON
char* convert_db_query(const char* query, const char* dbms, const Mapping_table* db_table);

#endif
