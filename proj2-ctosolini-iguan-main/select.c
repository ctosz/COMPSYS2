#include "select.h"

int select_folder(Details* info, int connfd, int* tag_num) {

    char select_buffer[BUFFERLEN_READ];
    char select_response[BUFFERLEN_READ]; // for select folder 


    sprintf(select_buffer, "A%d SELECT \"%s\"\r\n", *tag_num, info->folder);

    int wstat = write(connfd, select_buffer, strlen(select_buffer));
    if (wstat == -1) {
        printf("Error connecting to server: folder selection\n");
        exit(IMAP_ERROR); 
    }
    // printf("\nMessage Sent: %s", select);

    int rstat = read(connfd, select_response, BUFFERLEN_READ);
    if (rstat < 0) {
        printf("Error reading folder message\n");
        exit(IMAP_ERROR);
    }
    select_response[rstat] = '\0'; // null terminate
    //printf("%s\n", buffer3);

    // get last line of read info
    char* status = get_status_line(select_response);
    char* select_token = strtok(status, " "); // remove tag
    select_token = strtok(NULL, " "); // success/failure of read()

    // if folder was found:
    if (strcmp("OK", select_token) == 0) {
        return FOLDER_FOUND;
    }
    else {
        return FOLDER_NOT_FOUND;
    }
}

// iterate through read lines from SELECT folder until line with tag at the start i.e., status line 
char* get_status_line(char* buffer) {

    char* status_line = "";

    // check whether the first line has the A
    if (strncmp(buffer, "A", 1) == 0) {
        status_line = buffer;
        return status_line;
    }

    char* buf1 = strtok(buffer, "\n");
    while (strncmp(buf1, "A", 1) != 0) {
        //printf("buf1 = %s\n", buf1);
        status_line = buf1;
        buf1 = strtok(NULL, "\n");
    }
    return status_line;
}