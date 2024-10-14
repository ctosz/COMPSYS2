#include "retrieve.h"

void retrieve_email(int connfd, char* message_num, int* tag_num) {

    (*tag_num)++;
    size_t command_size;
    
    command_size = snprintf(NULL, 0, "A%d FETCH %s %s\r\n", *tag_num, message_num, MODE);
    char fetch_command[command_size];
    sprintf(fetch_command, "A%d FETCH %s %s\r\n", *tag_num, message_num, MODE);
    int wstat = write(connfd, fetch_command, strlen(fetch_command));
    if (wstat == -1) {
        fprintf(stderr, "Error writing to server: retrieve.\n");
        exit(IMAP_ERROR);
    }

    char* email_content = NULL;
    receive_email_content(connfd, &email_content);

    if (!email_content) {
        fprintf(stderr, "Failed to retrieve email content.\n");
        // free(email_content);
    }
    else {
        printf("%s", email_content);
        // free(email_content);
    }
    free(email_content);
    exit(0);
}

void receive_email_content(int connfd, char** email_content) {
    
    // Changing approach to use file pointers as per Ed discussion #772
    FILE* fp = fdopen(connfd, "r");
    if (!fp) {
        fprintf(stderr, "Error creating file pointer from file descriptor\n");
        exit(OTHER_ERROR);
    }

    char* line = NULL;
    size_t len = 0;

    // Read and discard the first line
    getline(&line, &len, fp);  

    // if the email is not found, print an error message and exit
    if (strncmp(line, "A", 1) == 0 && strstr(line, "BAD")) {
        printf("Message not found\n");
        exit(3);
    }
    
    // Read the email lengths in the fetch response line
    char* length_start = strstr(line, "{");
    char* length_end = strstr(line, "}");
    if (length_start == NULL || length_end == NULL) {
        fprintf(stderr, "Fetch response parse error\n");
        exit(IMAP_ERROR);
    }
    length_end = '\0';
    int email_length = atoi(length_start + 1);
    free(line);

    *email_content = (char*) malloc(email_length + 1);
    if (*email_content == NULL) {
        fprintf(stderr, "Error allocating memory for email content\n");
        exit(OTHER_ERROR);
    }

    fread(*email_content, 1, email_length, fp);
    fclose(fp);
}
