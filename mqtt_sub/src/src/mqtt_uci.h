#ifndef UCI_H
#define UCI_H
/**
 * @brief uci type config filename
*/
#define CONFIG "mqtt_sub"

/**
 * @brief max limit of topics to add
*/
#define MAXTOPICS 20      

/**
 * @brief max limit of events for each topic to add
*/
#define MAXEVENTS 20   

/**
 * @brief structure holds information for subscriber connection to broker
*/
struct subscriber {
        int port;
        int tls;
        char email[250];
        char cafile[250];
        char certfile[250];
        char keyfile[250];
        char localhost[250];
};
extern struct subscriber mqtt_subs;

/**
 * @brief structure holds information about each event
*/
struct event {
        char type[3];
        char value[250];
        char action[250];
        char target[250];
};

/**
 * @brief structure holds information about each topic
*/
struct topic {
        int ev_counter;
        char qos[3];
        char sub[3];
        char name[250];
        struct event *events[MAXEVENTS];
};
/**
 * @brief points to all topic struct pointers
*/
extern struct topic *topic_ptr[MAXTOPICS];   

/**
 * @brief amount of topics that exist
*/
extern int topic_counter;  

/**
 * @brief function initializes uci library, loads config file and saves its content if found
 * @param config_file - uci type config file
 * @return 0 success | 1 uci load fail | 2 wrong config file or its values
*/
int init_uci(const char *config_file);

/**
 * @brief releases allocated structs memory
 * @note call on program exit
*/
void clean_uci_data(void);


#endif