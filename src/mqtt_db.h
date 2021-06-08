#ifndef MQTTDB_H
#define MQTTDB_H

/**
 * @brief Check if database exists, if not create and insert default query
 * @return 0 success or database already exists | 1 failed to create db file |
 * 2 failed to open database | 3 error while inserting query
 * 
*/
int check_database(sqlite3 *DB, const char *db_path);

#endif