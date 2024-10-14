#include "login.h"

int imap_login(Details* info, int connfd, int* tag_num) {
    
    (*tag_num)++;
    char login_request[BUFFERLEN_READ];
    char login_response[BUFFERLEN_READ]; // for login status 

    sprintf(login_request, "A%d LOGIN %s %s\r\n", *tag_num, info->username, info->password);

    int wstat = write(connfd, login_request, strlen(login_request));
    if (wstat == -1) {
        printf("Error writing to server: login\n");
        exit(IMAP_ERROR); 
    }

    int rstat = read(connfd, login_response, BUFFERLEN_READ);
    if (rstat < 0) {
        printf("Error reading server message: login status\n");
        exit(IMAP_ERROR);
    }
    login_response[rstat] = '\0'; // null terminate 

    char* login_token = strtok(login_response, " "); // skip the tag
    login_token = strtok(NULL, " "); // get the status token

    if (strcmp("OK", login_token) == 0) {
        // login ok
        return LOGIN_OK;
    }
    else {
        return LOGIN_FAILED;
    }
}
