#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mosquitto.h>
#include <sqlite3.h>
#include <string.h>
#include <uci.h>
#include "sub_utils.h"
#include "mqtt_parse.h"
#include "mqtt_uci.h"
#include "mqtt_db.h"

sqlite3 *DB = NULL;

int subscribe_to_topics(struct mosquitto *mosq);

void sig_handler(int signum)
{       
        // Soft kill signal
        iter_signal = 0;
}

int init_proccesses(void)
{       
        /**
         * Start initialization processes:
         * register kill signal,
         * read config file,
         * open databse,
         * initialize mosquitto lib
        */

        fprintf(stdout, "%s\n", "Configuring subscriber...");

        signal(SIGTERM, sig_handler);

        if (init_uci(CONFIG) != 0) {
                fprintf(stderr, "%s\n", "Failed to initialize uci.");
                return UCI_IERR;
        }

        if (check_database(DB, DBPATH) != 0) {
                fprintf(stderr, "%s\n", "Failed to create new database file");
                return SQL_IERR;
        }

        if (sqlite3_open(DBPATH, &DB) != 0) {
                fprintf(stderr, "%s\n", "Failed to open database.");
                return SQL_IERR;
        }
                
        if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
                fprintf(stderr, "%s\n", "Failed to initialize mosquitto.");
                return MOSQ_IERR;
        }

        fprintf(stdout, "%s\n", "Successfully initialized subscriber");
        return INIT_SUCC;
}

void on_connect(struct mosquitto *mosq, void *data, int res) 
{
        /**
         * Custom callback for mosquitto
         * registered in set_callbacks()
         * Subscribes to all found topics
        */

        if (res || subscribe_to_topics(mosq)) {
                fprintf(stdout, "%s\n", "Failed to subscribe to any topic");
                iter_signal = 0;
        }
    
}

void on_message(struct mosquitto *mosq, void *data, const struct mosquitto_message *msg)
{
        /**
         * Custom callback for mosquitto
         * registered in set_callbacks()
         * saves received messages to database
        */

        fprintf(stdout,"new message with topic %s: %s \n", msg->topic, (char *) msg->payload);
        if (msg->topic != NULL && msg->payload != NULL) {

                int match = 0;
                char *query = NULL;

                for (int i = 0; i < topic_counter; i++) {
                        if (strcmp(msg->topic, topic_ptr[i]->name) == 0) {
                                query = prepare_query(  msg->topic, (char *) msg->payload, i);
                                match = 1;
                                break;
                        }      
                }
                
                if (match == 0)
                        query = prepare_query(  msg->topic, (char *) msg->payload, -1);

                if (query != NULL) {
                        if (sqlite3_exec(DB, query, 0, 0, 0) != 0){
                                fprintf(stdout, "%s\n", "Failed to insert message into DB");
                        }
                        
                        sqlite3_free(query);            
                }
        }
        else {
                fprintf(stdout, "%s\n", "Couldn't receive message from mosquitto publisher.");
          
        }
}

void set_callbacks(void)
{
        /**
         * Register custom callbacks to mosquitto functions
        */

        mosquitto_connect_callback_set(mosq, on_connect);
        mosquitto_message_callback_set(mosq, on_message);
}

int subscribe_to_topics(struct mosquitto *mosq)
{
        /**
         * Loop thru every provided topic
         * And try to subscribe to it
         * If any topic somehow is missing - skip to the next
        */

        int subscribed = 0;

        if (mosq == NULL || topic_ptr == NULL)
                return 1;



        for (int i = 0; i < topic_counter; i++) {

                if (topic_ptr[i] == NULL || topic_ptr[i]->sub == 0) 
                        continue;
                
                int _qos = strtol(topic_ptr[i]->qos, NULL, 10);

                mosquitto_subscribe(mosq, NULL, topic_ptr[i]->name, _qos);

                subscribed++;

                char buff[250];
                strncpy(buff, "Subscribed to topic: ", 22);
                strncat(buff, topic_ptr[i]->name, 227);
                fprintf(stdout,"%s: %s\n", "Subscribed to topic:", buff);
        }

      

        if (subscribed == 0)
                return 2;

        return 0;
}

int connect_to_broker(struct mosquitto *mosq)
{
        /**
         * Connects to mqtt broker with broker struct settings
         * Default settings - localhost / 1883
        */
       
        if (mosquitto_connect(mosq, mqtt_subs.localhost, mqtt_subs.port, 60)) {
                fprintf(stderr, "%s\n", "Failed to connect to mosquitto broker.");
                return BROKER_ERR;
        }
        
        fprintf(stdout, "Successfully connected to mosq broker at port: %d, host %s.\n", mqtt_subs.port, mqtt_subs.localhost);
        return BROKER_SUCC;
}

int activate_tls(struct mosquitto *mosq)
{
        if (mqtt_subs.tls != 1) 
                return TLS_ERR;
                
        if (mosquitto_tls_set(mosq, mqtt_subs.cafile, NULL, mqtt_subs.certfile, mqtt_subs.keyfile, NULL) != MOSQ_ERR_SUCCESS) {
                fprintf(stdout, "%s\n", "Failed to activate TLS for subscriber.");
                return TLS_ERR;
        }

 
        fprintf(stdout, "%s\n", "subscriber TLS activated.");
        
        return TLS_SUCC;
}

void clean_up(void) 
{       
        fprintf(stdout, "%s\n", "Subscriber exiting...");

        clean_uci_data();
        
        sqlite3_close(DB);

        mosquitto_destroy(mosq);

        mosquitto_lib_cleanup();
}

