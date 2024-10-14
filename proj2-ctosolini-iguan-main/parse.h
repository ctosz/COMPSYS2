#ifndef PARSE_H
#define PARSE_H

#include "universal.h"

#define FROM_TOK "From:"
#define TO_TOK "To:"
#define DATE_TOK "Date:"
#define SUBJECT_TOK "Subject:"

void parse_header(int connfd, char* message_num, int* tag_num);
void handle_fetch_command(int connfd, char* message_num, int* tag_num, char* header, char* header_type);
void handle_parse_response(char* response, char* field);

#endif //