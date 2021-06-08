#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mosquitto.h>
#include <sqlite3.h>
#include "sub_utils.h"

int iter_signal = 1;
struct mosquitto *mosq = NULL;

int main(void) 
{
        int ret = 0;
        
        // signal, mosquitto, database initialization
        if (init_proccesses() != INIT_SUCC) {
                ret = 1;
                goto clean;
        }
        
        // create client instance
        mosq = mosquitto_new(NULL, true, NULL);
        if (mosq == NULL) {  
                ret = 2;
                goto clean;
        }

        // Goes thru, doesn't impact program with either bad return
        activate_tls(mosq);
      
        // set callbacks for mosquitto loop
        set_callbacks();
       
        // Connect to mqtt broker
        if (connect_to_broker(mosq) != BROKER_SUCC) {
                ret = 4;
                goto clean;
        }
        
        while (iter_signal)
                mosquitto_loop_start(mosq);
        
        mosquitto_loop_stop(mosq, true);
        mosquitto_disconnect(mosq);

        clean:
                clean_up();

        return ret;
}

