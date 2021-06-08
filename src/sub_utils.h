#ifndef SUBUTILS_H
#define SUBUTILS_H

#define BROKER_SUCC 0
#define TLS_SUCC 0
#define TLS_ERR 1
#define BROKER_ERR 1
#define INIT_SUCC 0
#define UCI_IERR 1
#define SQL_IERR 2
#define MOSQ_IERR 3

#define DBPATH "/var/lib/mqtt_sub/data.db"

extern int iter_signal;
extern struct mosquitto *mosq;

/**
 * @brief Initializes signal, uci, database, mosquitto proccesses
 * @return On success INIT_SUCC | On failure UCI_IERR / SQL_IERR / MOSQ_IERR
*/
int init_proccesses(void);

/**
 * @brief sets connect, message callbacks to mosquitto
*/
void set_callbacks(void);

/**
 * @brief Activates tls with default certificate
 * @return On success TLS_SUCC | On failure TLS_ERR
*/
int activate_tls(struct mosquitto *mosq);

/**
 * @brief Connects sto mqtt broker with given or default values
 * @param @struct *mosq
 * @note Default port - 1883 / host - localhost
 * @return On success BROKER_SUCC | On failure BROKER_ERR
*/
int connect_to_broker(struct mosquitto *mosq);

/**
 * @brief Releases allocated memory and closes database
*/
void clean_up(void);

#endif