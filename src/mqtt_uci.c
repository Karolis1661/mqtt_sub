#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include "mqtt_uci.h"

struct subscriber mqtt_subs = {.port = 1883, .tls = 0, .localhost = "localhost"};
struct topic *topic_ptr[MAXTOPICS];     

int topic_counter = 0;        

static int save_config_vals(struct uci_context *ctx, struct uci_section *s);
static int read_subscriber(struct uci_context *ctx, struct uci_section *s);
static int read_topic(struct uci_context *ctx, struct uci_section *s);
static int read_event(struct uci_context *ctx, struct uci_section *s);

int init_uci(const char *config_file)
{
        /**
         * Initialize uci library
         * Loop thru all config file sections
         * Look for declared values
         * Save to struct
        */

        int ret = 0;

        struct uci_element *i_iter = NULL;
        struct uci_package *package = NULL;
        struct uci_context *context = uci_alloc_context();

        if (uci_load(context, config_file, &package) != UCI_OK) {
                fprintf(stderr, "%s\n", "Failed to load uci function.");
                ret = 1;
                goto cleanup;
        }

        uci_foreach_element(&package->sections, i_iter) {

                struct uci_section *section = uci_to_section(i_iter);

                if (save_config_vals(context, section) == 1) {
                        fprintf(stderr, "%s\n", "Failed to read config file");
                        ret = 2;
                        break;
                }
        }

        cleanup:
                uci_free_context(context);

        return ret;
}

static int save_config_vals(struct uci_context *ctx, struct uci_section *s)
{
        int ret = 0;

        if (strcmp(s->type, "subscriber") == 0)
                ret = read_subscriber(ctx, s);

        else if (strcmp(s->type, "topic") == 0)
                ret = read_topic(ctx, s);

        else if (strcmp(s->type, "event") == 0)
                ret = read_event(ctx, s);
        
    return ret;
}

static int read_subscriber(struct uci_context *ctx, struct uci_section *s)
{
        /**
         * @brief reac subscriber section at config file
         * and check for main and additional cert values
         * @return 0 - success on all values | 1 - main values wasn't found | 2 certfile values wasn't found
        */

        // Look for specified values
        const char *host = uci_lookup_option_string(ctx, s, "host");
        const char *port = uci_lookup_option_string(ctx, s, "port");
        const char *tls = uci_lookup_option_string(ctx, s, "tls");
        const char *email = uci_lookup_option_string(ctx, s, "email");
        const char *cafile = uci_lookup_option_string(ctx, s, "cafile");
        const char *certfile = uci_lookup_option_string(ctx, s, "certfile");
        const char *keyfile = uci_lookup_option_string(ctx, s, "keyfile");
     
        // Check if main specified values are valid
        if (host == NULL || host[0] == '\0' || port == NULL || port[0] == '\0' || tls == NULL || tls[0] == '\0')
                return 1;
        
        // Save main values
        strncpy(mqtt_subs.localhost, host, 248);
        mqtt_subs.tls = strtol(tls, NULL, 10);
        mqtt_subs.port = strtol(port, NULL, 10);
        
        if (email != NULL)
                strncpy(mqtt_subs.email, email, 249);
        
       
        // Then if there's cert files
        if (cafile == NULL || cafile[0] == '\0' || certfile == NULL || certfile[0] == '\0' || keyfile == NULL || keyfile[0] == '\0') 
                return 2;
        
        // Save certfiles
        strncpy(mqtt_subs.cafile, cafile, 249);
        strncpy(mqtt_subs.certfile, certfile, 249);
        strncpy(mqtt_subs.keyfile, keyfile, 249);

        return 0;
}

static int read_topic(struct uci_context *ctx, struct uci_section *s)
{
        /**
         * @brief Read topic section at config file
         * @return 0 success | 1 maximum amount of topics achieved | 2 config values wasn't found
        */
        
        if ((topic_counter + 1) == MAXTOPICS)
                return 1;

        // Look for specified values
        const char *topic_name = uci_lookup_option_string(ctx, s, "name");
        const char *subscription = uci_lookup_option_string(ctx, s, "subscription");
        const char *qos = uci_lookup_option_string(ctx, s, "qos");

         // Check if given input is valid
        if (topic_name == NULL || topic_name[0] == '\0' || subscription == NULL || subscription[0] == '\0' ||
            qos == NULL || qos[0] == '\0') 
                return 2;

        // Malloc memory for current topic
        topic_ptr[topic_counter] = malloc(sizeof (struct topic));
        if (topic_ptr[topic_counter] == NULL)
                return 3;
        
        strncpy(topic_ptr[topic_counter]->name, topic_name, 249);
        strncpy(topic_ptr[topic_counter]->qos, qos, 2);
        strncpy(topic_ptr[topic_counter]->sub, subscription, 2);
        topic_ptr[topic_counter]->ev_counter = 0;

        topic_counter++;

        return 0;
}

static int read_event(struct uci_context *ctx, struct uci_section *s){

        int topic_index = -1;  // Store topic index for which this even will belong

        // Try to get specified values from event section at config file
        const char *topic = uci_lookup_option_string(ctx, s, "topic");   
        const char *target = uci_lookup_option_string(ctx, s, "target");
        const char *action = uci_lookup_option_string(ctx, s, "action");
        const char *value = uci_lookup_option_string(ctx, s, "value");
        const char *type = uci_lookup_option_string(ctx, s, "type");

        // check if values are valid
        if (topic == NULL || topic[0] == '\0' || target == NULL || target[0] == '\0' || action == NULL ||
            value == NULL || value[0] == '\0' || type == NULL || type[0] == '\0') {
                return 1;
        }

        // Compare topic name with topic name in event section
        for (int i = 0; i < topic_counter; i++) {
                if (strcmp(topic_ptr[i]->name, topic) == 0)
                        topic_index = i;
        }

        // Topic to which this even belongs wasn't found
        if (topic_index == -1)
                return 2;

        // Check if topic doesn't reached its events limit
        if ((topic_ptr[topic_index]->ev_counter + 1) == MAXEVENTS)
                return 3;

        int e_ind = topic_ptr[topic_index]->ev_counter; // Current event index to be malloced

        // Malloc memory for event in current topic
        topic_ptr[topic_index]->events[e_ind] = malloc(sizeof(struct event));
        if (topic_ptr[topic_index]->events[e_ind] == NULL)
                return 4;
        
        strncpy(topic_ptr[topic_index]->events[e_ind]->action, action, 249);
        strncpy(topic_ptr[topic_index]->events[e_ind]->target, target, 249);
        strncpy(topic_ptr[topic_index]->events[e_ind]->type, type, 2);
        strncpy(topic_ptr[topic_index]->events[e_ind]->value, value, 249);

        // New event was added
        topic_ptr[topic_index]->ev_counter++;
        return 0;
}

void clean_uci_data(void)
{
        /**
         * @brief free memory by looping thru each topic and thru each event in that topic
        */

        for (int i = 0; i < topic_counter; i++) {
                 
                 // If topic has events, release them
                 if (topic_ptr[i]->ev_counter > 0) {

                         for (int j = 0; j < topic_ptr[i]->ev_counter; j++) {
                                 
                                if (topic_ptr[i]->events[j] != NULL)
                                        free(topic_ptr[i]->events[j]);

                         }
                 }

                if (topic_ptr[i] != NULL)
                        free(topic_ptr[i]);

        }
}