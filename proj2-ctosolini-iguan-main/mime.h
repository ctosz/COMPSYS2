#include <string.h>
#ifndef MIME_H
#define MIME_H

#include "universal.h"
#include "retrieve.h"

#define MIME_TOKEN " boundary="
#define MIME_VERSION "MIME-Version: "
#define CONTENT_TYPE "Content-Type: "
#define CORRECT_MIME_VERSION "1.0"
#define CORRECT_MEDIA_TYPE "multipart/alternative"
#define DOUBLE_QUOTE '\"'
#define START_BODY "\r\n --"
#define START_TAG '<'
#define END_TAG '>'
#define NUM_HEADERS_TO_CHECK 2
#define NUM_CHARS_TO_COMP 11
#define HEADERS_FIRST_LETTER "C"


#define CTE_QUOTED_PRINCIPLE "Content-Transfer-Encoding: quoted-printable"
#define CTE_7BIT "Content-Transfer-Encoding: 7bit"
#define CTE_8BIT "Content-Transfer-Encoding: 8bit"
#define CONTENT_TYPE_BODY "Content-Type: text/plain; charset=UTF-8"
#define CHARSET

void decode_mime(int connfd, char* message_num, int* tag_num);
char* match_headers(int connfd, char* message_num, int* tag_num);
char* remove_quotes(char* str);
void unfold_headers(char* email_body);
void retrieve_contents_decoded(int connfd, char* message_num, int* tag_num, char* boundary_param);
void check_param(char* line, int* seen_headers);
#endif // MIME_H
