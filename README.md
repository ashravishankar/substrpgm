# SubstrPgm - SQL Query Translation Tool

A C program that converts SQL queries between different database management systems (DBMS).

## What it does

Translates generic SQL function calls to database-specific syntax using a JSON configuration file. For example:
- `CMD_SUBSTRING` → `substring` (PostgreSQL) or `substr` (SQLite/MySQL)
- `CMD_LENGTH` → `char_length` (PostgreSQL) or `length` (SQLite/MySQL)

## Install Libraries

```bash
sudo apt-get install sqlite3 libsqlite3-dev libcjson-dev
```

## Build

```bash
make clean && make
```

## Usage

```bash
# Convert query for PostgreSQL 
./substrpgm --database PostgreSQL --query "SELECT CMD_SUBSTRING(name,1,3) FROM employees;"

[PostgreSQL] Converted Query:
SELECT substring(name,1,3) FROM employees;

# Convert query for DBMS3
$ ./substrpgm --database DBMS3 --query "SELECT CMD_SUBSTRING(name,1,3) FROM employees;"

[DBMS3] Converted Query:
SELECT sbstr(name,1,3) FROM employees;

# Convert and execute on SQLite database
$ ./substrpgm --database sqlite --query "SELECT CMD_SUBSTRING(name,1,3) FROM employees;" --execute emp.db

[sqlite] Converted Query:
SELECT substr(name,1,3) FROM employees;

Executing query on SQLite:
SELECT substr(name,1,3) FROM employees;

substr(name,1,3)
----------------------------------------------------------------
Ada
Bar
Cat

# List supported databases
./substrpgm --list-databases

Supported database engines (from configuration):
  - DBMS1
  - DBMS2
  - DBMS3
  - PostgreSQL
  - MySQL
  - sqlite

# Export to file
./substrpgm --database PostgreSQL --query "SELECT CMD_SUBSTRING(name,1,3) FROM employees;" --export output.sql

Exported converted query to output.sql
$ cat output.sql
SELECT substring(name,1,3) FROM employees;

# Specify config file
$ ./substrpgm --database sqlite --query "SELECT CMD_SUBSTRING(name,1,3) FROM employees;" --config config/cu
stom_config.json

[sqlite] Converted Query:
SELECT substr(name,1,3) FROM employees;

```

## Configuration

Database function mappings are defined in `config/config.json`. Add new functions or databases by editing this file.

## Testing

```bash
make clean-test && make test

./test_runner
**** SubstrPgm Unit Tests ***

--- Running test_query_sqlite ---
PASS: Test mapping table created successfully
PASS: Query conversion returned non-NULL result
PASS: Query contains 'substr' function
PASS: Query no longer contains 'CMD_SUBSTRING'
Input:  SELECT CMD_SUBSTRING(name, 1, 3) FROM users
Output: SELECT substr(name, 1, 3) FROM users
✓ test_query_sqlite PASSED

--- Running test_multiple_functions ---
PASS: Test mapping table created successfully
PASS: Query conversion returned non-NULL result
PASS: Query contains 'substring' function
PASS: Query contains 'char_length' function
PASS: Query no longer contains 'CMD_SUBSTRING'
PASS: Query no longer contains 'CMD_LENGTH'
Input:  SELECT CMD_SUBSTRING(name, 1, CMD_LENGTH(name)) FROM users
Output: SELECT substring(name, 1, char_length(name)) FROM users
✓ test_multiple_functions PASSED

--- Running test_unknown_database ---
PASS: Test mapping table created successfully
PASS: Query conversion returned non-NULL result
PASS: Query unchanged for unknown database
Input:  SELECT CMD_SUBSTRING(name, 1, 3) FROM users
Output: SELECT CMD_SUBSTRING(name, 1, 3) FROM users
✓ test_unknown_database PASSED

--- Running test_null_inputs ---
PASS: Test mapping table created successfully
Invalid arguments to convert_query
PASS: NULL query input handled correctly
Invalid arguments to convert_query
PASS: NULL database input handled correctly
Invalid arguments to convert_query
PASS: NULL table input handled correctly
✓ test_null_inputs PASSED

--- Running test_config_loading ---
PASS: Configuration file loaded successfully
PASS: Configuration contains mappings
PASS: Command mappings allocated
Loaded 3 command mappings from config file
PASS: Query conversion with loaded config works
Real config test - Input:  SELECT CMD_SUBSTRING(name, 1, 3) FROM users
Real config test - Output: SELECT substring(name, 1, 3) FROM users
✓ test_config_loading PASSED

=== Test Summary ===
Total tests: 5
Passed: 5
Failed: 0
Success rate: 100.0%
```

## Dependencies

- gcc
- sqlite3
- libsqlite3-dev
- libcjson-dev

## Files

- `src/` - Source code
- `include/` - Header files  
- `config/` - JSON configuration
- `test/` - Unit tests
