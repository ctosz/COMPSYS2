#ifndef LIST_H
#define LIST_H

#include "universal.h"
#include "parse.h"

void list_subjects(int connfd, int* tag_num);
void fetch_subject_content(int connfd, char* message_num, int* tag_num);

#endif // LIST_H