#ifndef RETRIEVE_H
#define RETRIEVE_H

#include "universal.h"

#define ERROR -1
#define MODE "BODY.PEEK[]"

void retrieve_email(int connfd, char* message_num, int* tag_num);
void receive_email_content(int connfd, char** email_content);

#endif // RETRIEVE_H