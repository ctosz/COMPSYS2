#ifndef SELECT_H
#define SELECT_H
#define FOLDER_FOUND 1
#define FOLDER_NOT_FOUND -1

#include "universal.h"

int select_folder(Details* info, int connfd, int* tag_num);
char* get_status_line(char* buffer);

#endif 