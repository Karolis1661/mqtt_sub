#ifndef MAIL_H
#define MAIL_H

/**
 * @brief Event message to send, holds 1000 bytes.
 * Use with strncpy
*/
extern char em_message[1000];

/**
 * @brief email pointer
*/
extern const char *email;


int send_email(void);

#endif