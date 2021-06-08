#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "mqtt_mail.h"

/*
 * For an SMTP example using the multi interface please see smtp-multi.c.
 */
 
/* The libcurl options want plain addresses, the viewable headers in the mail
 * can very well get a full name as well.
 */

// Source - https://curl.se/libcurl/c/smtp-ssl.html

char em_message[1000];
const char *email;

static const char *payload_text[] = {
        "Subject: mqtt event\r\n",
        "\r\n",
        em_message, 
        "\r\n", 
        NULL
};

static struct upload_status {
        int lines_read;
};
 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
        struct upload_status *upload_ctx = (struct upload_status *)userp;
        const char *data;
      
        if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
          return 0;
        }
      
        data = payload_text[upload_ctx->lines_read];
      
        if(data) {
          size_t len = strlen(data);
          memcpy(ptr, data, len);
          upload_ctx->lines_read++;
      
          return len;
        }
      
        return 0;
}
 
int send_email(void)
{
        CURL *curl;
        CURLcode res = CURLE_OK;
        struct curl_slist *recipients = NULL;
        struct upload_status upload_ctx;
        
        upload_ctx.lines_read = 0;
        
        curl = curl_easy_init();
        if(curl) {
                fprintf(stdout, "%s\n", "Preparing email content");
                fprintf(stdout, "%s %s\n", "Receiver address:", email);
                /* This is the URL for your mailserver */
                curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
            
                /* Note that this option isn't strictly required, omitting it will result
                * in libcurl sending the MAIL FROM command with empty sender data. All
                * autoresponses should have an empty reverse-path, and should be directed
                * to the address in the reverse-path which triggered them. Otherwise,
                * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
                * details.
                */
                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "MQTT");
            
                /* Add two recipients, in this particular case they correspond to the
                * To: and Cc: addressees in the header, but they could be any kind of
                * recipient. */
                recipients = curl_slist_append(recipients, email);
                curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
            
                curl_easy_setopt(curl, CURLOPT_USERNAME, "mqtt.event@gmail.com");
                curl_easy_setopt(curl, CURLOPT_PASSWORD, "Ziedas321");

                /* We're using a callback function to specify the payload (the headers and
                * body of the message). You could just use the CURLOPT_READDATA option to
                * specify a FILE pointer to read from. */
                curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
                curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            
                curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                /* Send the message */
                res = curl_easy_perform(curl);
            
                /* Check for errors */
                if(res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                curl_easy_strerror(res));
            
                /* Free the list of recipients */
                curl_slist_free_all(recipients);
            
                /* curl won't send the QUIT command until you call cleanup, so you should
                * be able to re-use this connection for additional messages (setting
                * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
                * curl_easy_perform() again. It may not be a good idea to keep the
                * connection open for a very long time though (more than a few minutes
                * may result in the server timing out the connection), and you do want to
                * clean up in the end.
                */
                curl_easy_cleanup(curl);

                
        }
        
        return (int)res;
}