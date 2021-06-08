#include <stdio.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include "mqtt_db.h"

static int db_exists(const char *filename){
        /**
         * @brief check if database file exists
         * @return -1 doesn't exist | 0 exists
        */
    
        struct stat buffer;

        int ret = stat(filename, &buffer);

        return ret;
}

static int create_db(const char *dbpath)
{
        /**
         * @brief create database file
         * @return 0 success | 1 failure
        */

        FILE *ptr;

        ptr = fopen(dbpath, "w") ;
        if (ptr == NULL)
                return 1;
        
        fclose(ptr);
        return 0;
}

int check_database(sqlite3 *DB, const char *db_path)
{
        int ret = 0;

        if (db_exists(db_path) == 0) {
                return 0;
        }
                
        fprintf(stdout, "%s\n", "Creating new database file for mqtt subscriber.");

        if (create_db(db_path) != 0) {
                fprintf(stderr, "%s\n", "Failed to create database file for mqtt subscriber.");
  
                return 1;
        }
                
        
        const char *query = "CREATE TABLE Messages ( \
                            id integer primary key autoincrement not null,\
                            Topic varchar(250),\
                            Message varchar(250),\
                            Time timestamp default (datetime('now', 'localtime')) not null);";

        if (sqlite3_open(db_path, &DB) != 0) {
                fprintf(stderr, "%s\n", "Failed to open database.");
                return 2;
        }

        char *err = NULL;
        sqlite3_exec(DB, query, 0, NULL, &err);

        // Error while executing insert statement, print out and close db.
        if (err != NULL) {
                fprintf(stderr, "%s\n", err);
                ret = 3;
                goto close;
        }
                
        close:
                sqlite3_close(DB);

        puts("Successfully created database");
        return ret;
}