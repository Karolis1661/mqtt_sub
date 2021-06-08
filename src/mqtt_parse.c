#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include <sqlite3.h>
#include "mqtt_uci.h"
#include "mqtt_mail.h"


static char *is_valid_json(const char *msg, int struct_ind);
static int check_json_value(const char *str, const char *action, const char *target, const char *type);
static int int_val_compare(const char *val1, const char *val2, const char *action);
static int str_val_compare(const char *val1, const char *val2, const char *action);

char *prepare_query(const char *topic, char *msg, int struct_ind)
{
        /**
         * Check if message query is valid json or string
        */

        /**
         * Found message with wildcard topic, return instantly
        */
        if (struct_ind == -1) {
                char *query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');", topic, msg);
                return query;
        }

        int succ = 0;
        char *msg2 = NULL;
        
        fprintf(stdout, "%s\n", "Received message from mqtt_publisher.");

        if (topic == NULL || msg == NULL) {
                fprintf(stdout, "%s\n", "Received message or topic is NULL.");
                return NULL;
        }
        
        if (topic[0] == '\0' || msg[0] == '\0') {
                fprintf(stdout, "%s\n", "Received message or topic is empty.");
                return NULL;
        }

        if (json_tokener_parse(msg) != NULL) {
                fprintf(stdout, "%s\n", "Message from mqtt_publisher is valid json format. Examining...");

                msg2 = is_valid_json(msg, struct_ind);
                if (msg2 == NULL) {
                        fprintf(stdout, "%s\n","new json message is null");
                }
                else {
                        /**
                         * Event condition was fullfiled, send email with information 
                        */
                        strncpy(em_message, "Event triggered at topic: ", 27);
                        strncat(em_message, topic, 100);
                        strncat(em_message, " with message: ", 16);
                        strncat(em_message, msg2, 700);
                        strncat(em_message, ".", 2);
                        email = mqtt_subs.email;
                        
                        if (email != NULL)
                                send_email();

                        succ = 1;
                }     
        }
        
        char *query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');", topic, msg);
        
        if (succ && msg2 != NULL)
                free(msg2);

        if (query == NULL) {
                fprintf(stdout, "%s\n", "Failed to prepare mqtt_publisher query for database.");
                return NULL;
        }

        return query;
}

static char *is_valid_json(const char *msg, int struct_ind)
{
        struct json_object *parsed_json;
        struct json_object *event;
        int found = 0;
        int index = 0;

        char *buffer = malloc (sizeof(char) * 1000);
        if (buffer == NULL)
                return NULL;

        parsed_json = json_tokener_parse(msg);
        if (parsed_json == NULL) {
                fprintf(stdout, "%s\n", "Failed to parse json from mqtt_publisher");
                return NULL;
        }
        
        // Check if received json has event which is specified at topic
        for (unsigned int i = 0; i < topic_ptr[struct_ind]->ev_counter; i++) {
                if (json_object_object_get_ex(parsed_json, topic_ptr[struct_ind]->events[i]->target, &event)) {
                        found = 1;
                        index = i;
                        break;
                }
        }
        
        if (!found) {
                fprintf(stdout, "%s\n", "No event matching at json object for given topic.");
                return NULL;
        }

        const char *target_var = topic_ptr[struct_ind]->events[index]->target;          // 'temp'
        const char *event_action = topic_ptr[struct_ind]->events[index]->action;        // '<' '>' '=='
        const char *target_val = topic_ptr[struct_ind]->events[index]->value;           // 50
        const char *target_type = topic_ptr[struct_ind]->events[index]->type;           // '0' '1'

        const char *val = json_object_get_string(event);                                // value from message

        // Compare new value with event rules
        if (check_json_value(val, event_action, target_val, target_type) == 0) {
                fprintf(stdout, "%s\n", "No conditions satisfied from examined json message.");
                return NULL;
        }

        strncpy(buffer, target_var, 400);
        strncat(buffer, ":", 2);
        strncat(buffer, val, 400);

        fprintf(stdout, "%s\n", "Json successfully parsed, saving values.");
        return buffer;
}

static int check_json_value(const char *str, const char *action, const char *target, const char *type)
{
        /**
         * @brief Check if string or int action required and compare
         * @return 0 if no match | 1 if lower, 2 if higher, 3 if equal
        */

        int ret = 0;

        if (str == NULL || action == NULL || target == NULL || type == NULL) {
                fprintf(stdout, "%s\n", "Received json message has null values.");
                return 0;
        }

        if (strcmp(type, "0") == 0)
                ret = int_val_compare(str, target, action);

        else if (strcmp(type, "1") == 0) 
                ret = str_val_compare(str, target, action);
        
        return ret;
}

static int int_val_compare(const char *val1, const char *val2, const char *action)
{
        /**
         * @brief Compare int values by given comparison rule 
         * @return 0 if no match | 1 if lower, 2 if higher, 3 if equal
        */

        int given_value = strtol(val1, NULL, 10);
        int target_value = strtol(val2, NULL, 10);
     
        int ret = 0;

        if (strcmp(action, "<") == 0) {
                fprintf(stdout, "Checkin %d %s %d \n", given_value, action, target_value );
                if (given_value < target_value)
                        ret = 1;
                    
        }
    
        else if (strcmp(action, ">") == 0) {
                fprintf(stdout, "Checkin %d %s %d \n", given_value, action, target_value );
                if (given_value > target_value) 
                        ret = 2;          
        }
                
        else if(strcmp(action, "==") == 0) {
                fprintf(stdout, "Checkin %d %s %d \n", given_value, action, target_value );
                if (given_value == target_value)
                        ret = 3;         
        }
               
        return ret;
}

static int str_val_compare(const char *val1, const char *val2, const char *action)
{
        /**
         * @brief compare if strings are equal
        */

        if (strcmp(action, "==") != 0)
                return 0;

        if (strcmp(val1, val2) == 0) {
                fprintf(stdout, "Checking if %s == %s\n", val1, val2);
                return 1;
        }
                

        return 0;        
}