#ifndef MQTTPARSE_H
#define MQTTPARSE_H

/**
 * @brief Prepares received message from mosquitto_pub for database query
 * If received json format - parses for additional conditions given by user
 * @return on success returns prepared query | on failure - null
*/
char *prepare_query(const char *topic, char *msg, int struct_ind);

#endif