#include "list.h"

void list_subjects(int connfd, int* tag_num) {

    (*tag_num)++;

    // request all message numbers in the current mailbox
    char request_message_nums[BUFFERLEN_READ];
    sprintf(request_message_nums, "A%d SEARCH ALL\r\n", *tag_num);
    int wstat = write(connfd, request_message_nums, strlen(request_message_nums));
    if (wstat == -1) {
        fprintf(stderr, "Error writing to server: get_message_num\n");
        exit(IMAP_ERROR);
    }

    char message_nums_response[BUFFERLEN_READ];
    int rstat = read(connfd, message_nums_response, BUFFERLEN_READ);
    if (rstat == -1) {
        fprintf(stderr, "Error reading from server: get_message_num\n");
        exit(IMAP_ERROR);
    }

    // exclude the server tagged response
    char* first_line = strstr(message_nums_response, CRLF);
    int search_result_len = first_line - message_nums_response;
    char search_result[search_result_len + 1]; // +1 for null terminator
    strncpy(search_result, message_nums_response, search_result_len);
    search_result[search_result_len] = '\0';
    //printf("Search result: %s\n", search_result);

    // get the message numbers
    int email_count = 0;
    char* message_num = strtok(search_result, " ");
    while(message_num != NULL) {
        if (isdigit(*message_num)) {
            printf("%s:", message_num);
            fetch_subject_content(connfd, message_num, tag_num);
            email_count++;
        }
        message_num = strtok(NULL, " ");
    }
    if (email_count == 0) {
        exit(0);
    }
}

void fetch_subject_content(int connfd, char* message_num, int* tag_num) {

    (*tag_num)++;

    char fetch_command[BUFFERLEN_READ];
    sprintf(fetch_command, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", *tag_num, message_num);
    int wstat = write(connfd, fetch_command, strlen(fetch_command));
    if (wstat == -1) {
        fprintf(stderr, "Error writing to server: fetch_subject_content\n");
        exit(IMAP_ERROR);
    }

    char fetch_response[BUFFERLEN_READ];
    int rstat = read(connfd, fetch_response, BUFFERLEN_READ);
    if (rstat == -1) {
        fprintf(stderr, "Error reading from server: fetch_subject_content\n");
        exit(IMAP_ERROR);
    }

    fetch_response[rstat] = '\0';

    handle_parse_response(fetch_response, SUBJECT_TOK);

}