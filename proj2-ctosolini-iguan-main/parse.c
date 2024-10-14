#include "parse.h"

void parse_header(int connfd, char* message_num, int* tag_num) {
    handle_fetch_command(connfd, message_num, tag_num, FROM_TOK, "FROM");
    handle_fetch_command(connfd, message_num, tag_num, TO_TOK, "TO");
    handle_fetch_command(connfd, message_num, tag_num, DATE_TOK, "DATE");
    handle_fetch_command(connfd, message_num, tag_num, SUBJECT_TOK, "SUBJECT");
}

void handle_fetch_command(int connfd, char* message_num, int* tag_num, char* header, char* header_type) {

    // increment the tag number
    (*tag_num)++;

    char write_buffer[BUFFERLEN_READ];
    char read_buffer[BUFFERLEN_READ];
    int rstat, wstat;
    sprintf(write_buffer, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (%s)]\r\n", *tag_num, message_num, header_type);
    wstat = write(connfd, write_buffer, strlen(write_buffer));
    if (wstat == -1) {
        fprintf(stderr, "Error writing to server: parse\n");
        exit(IMAP_ERROR);
    }
    rstat = read(connfd, read_buffer, BUFFERLEN_READ);
    read_buffer[rstat] = '\0';
    printf("%s", header);
    handle_parse_response(read_buffer, header);
}

void handle_parse_response(char* response, char* field) {

    // convert the response to lowercase
    char* response_lower = strdup(response);
    for (char* c = response_lower; *c; c++) {
        *c = tolower(*c);
        // c = NULL;
    }

    // convert the field to lowercase
    char* field_lower = strdup(field);
    for (char* c = field_lower; *c; c++) {
        *c = tolower(*c);
        // c = NULL;
    }

    
    char* start = strstr(response_lower, field_lower);
    if (start == NULL) {
        if (strcasecmp(field, TO_TOK) == 0) {
            printf("\n"); 
            return;
        }
        else if (strcasecmp(field, SUBJECT_TOK) == 0) {
            printf(" <No subject>\n");
            return;
        } else {
            fprintf(stderr, "%s field not found\n", field);
            return;
        }
    }

    // get the content of the original response
    start = response + (start - response_lower) + strlen(field);

    // unfolding is "simply removing any CRLF that is immediately followed by WSP"
    char unfolded[BUFFERLEN_READ];

    char* read = start;
    char* write = unfolded;

    while (*read) {
        if (*read == '\r' && *(read + 1) == '\n' && isspace(*(read + 2))) {
            read += strlen(CRLF); // skip the CRLF
        }
        else if (*read == '\n' && isspace(*(read + 1))) {
            read++; // skip the LF
        }

        // if the LF is not followed by a white space, we have hit the fetch response
        else if (*read == '\n') {
            break;
        }
        else {
            *write++ = *read++;
        }
    }
    *write = '\0';

    // remove trailing CR
    if (write > unfolded && *(write - 1) == '\r') {
        *(write - 1) = '\0';
    }

    printf("%s\n", unfolded);

    free(field_lower);
    free(response_lower);

    read = write = start = NULL;

}