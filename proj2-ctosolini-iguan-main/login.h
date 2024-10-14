#ifndef LOGIN_H
#define LOGIN_H

#define LOGIN_OK 1
#define LOGIN_FAILED -1

#include "universal.h"

int imap_login(Details* info, int connfd, int* tag_num);

#endif 