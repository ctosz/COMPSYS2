#define _POSIX_C_SOURCE 200112L
#define PORT "143"
#define TLS_PORT "993"
#define GET_INFO_SUCCESS 0
#define CREATION_FAILURE -1
#define USE_TCP 0
#define USE_TLS 1
#define MIN_CMDLN_ARGS 7
#define MAX_CMDLN_ARGS 12
#define MANDATORY_ARGS 3
#define ARG_PAIR 2
#define DEFAULT_FOLDER "INBOX"
#define DEFAULT_MESSAGE_NUM "*"

#include "login.h"
#include "select.h"
#include "retrieve.h"
#include "parse.h"
#include "mime.h"
#include "list.h"

#include <openssl/ssl.h>
#include <openssl/err.h>


void init_details(Details* info);

// make createsocket, login & select their own functions so main is not so long
int main(int argc, char* argv[]) {
    // hi
    // commandline args must be at least 7 and no more than 12
    if ((argc < MIN_CMDLN_ARGS) | (argc > MAX_CMDLN_ARGS)) {
        fprintf(stderr, "%d is an Incorrect Number of Commandline Arguments\n", argc);
        exit(COMMANDLINE_PARSING_ERROR);
    }

    int opt;
    Details* info = (Details*)malloc(sizeof(Details));
    init_details(info);

    int args_read = 0;
    // flags can be out of order, second last will always be command, last will always be servername 
    while ((opt = getopt(argc, argv, "u:p:f:n:t:"))!= -1) {

        switch (opt) {
            case 'u':
                // username 
                info->username = optarg;
                args_read += ARG_PAIR;
                break;
            case 'p':
                // password
                info->password = optarg;
                args_read += ARG_PAIR;
                break;
            case 'f':
                // folder
                info->folder = optarg;
                args_read += ARG_PAIR;
                break;
            case 'n':
                // message number for retrieval 
                info->message_num = optarg;
                args_read += ARG_PAIR;
                break;
            case 't':
                // use TLS instead of TCP 
                info->TLS = USE_TLS;
                args_read++; 
                break;
        }
    }

    info->command = argv[argc - 2]; // second last arg
    info->server_name = argv[argc - 1]; // last arg
    args_read += MANDATORY_ARGS;

    if (info->username == NULL) {
        fprintf(stderr, "Username not provided\n");
        exit(COMMANDLINE_PARSING_ERROR);
    }

    if (info->password == NULL) {
        fprintf(stderr, "Password not provided\n");
        exit(COMMANDLINE_PARSING_ERROR);
    }

    if (args_read > argc) {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(COMMANDLINE_PARSING_ERROR);
    }

    // 1. create socket: client
    int connfd = 0, get_set_of_socket_addresses;

    // set up address information 
    // from lecture slides lecture week 7 
    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED;


    get_set_of_socket_addresses = getaddrinfo(info->server_name, PORT, &hints, &result);

    if (get_set_of_socket_addresses != GET_INFO_SUCCESS) {
        fprintf(stderr, "Unable to establish connection\n");
        exit(CONNECTION_INITIATION_ERROR);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        connfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        // FAILURE
        if (connfd == CREATION_FAILURE) {
            continue; // skip to next available ip address
        }

        // success: try connect 
        if (connect(connfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            // connected!
            break;
        }
        close(connfd);
    }

    if (rp == NULL) {
        exit(CONNECTION_INITIATION_ERROR);
    }

    freeaddrinfo(result);

    // login to IMAP: write to the server 
    // generate a tag 
    int tag_num = 0; 
    char greeting_buf[BUFFERLEN_READ];
    int rstat = read(connfd, greeting_buf, BUFFERLEN_READ);
    if (rstat < 0) {
        fprintf(stdout, "Error reading server message: initialising connection\n");
        exit(CONNECTION_INITIATION_ERROR);
    }

    if (imap_login(info, connfd, &tag_num) == LOGIN_FAILED) {
        fprintf(stdout, "Login failure\n");
        exit(IMAP_ERROR); // exit with status 3 as per spec 
    }
    // select folder 

    // if folder was not found: use the default folder
    if (select_folder(info, connfd, &tag_num) == FOLDER_NOT_FOUND) {
        fprintf(stdout, "Folder not found\n");
        exit(IMAP_ERROR);
    }
    
    if (info->message_num == NULL) {
        info->message_num = DEFAULT_MESSAGE_NUM;
    }

    if (strcmp(info->command, "retrieve") == 0) {
        retrieve_email(connfd, info->message_num, &tag_num);
    }

    if (strcmp(info->command, "parse") == 0) {
        parse_header(connfd, info->message_num, &tag_num);
    }

    if (strcmp(info->command, "mime") == 0) {
        decode_mime(connfd, info->message_num, &tag_num);
    }

    if (strcmp(info->command, "list") == 0) {
        list_subjects(connfd, &tag_num);
    }


    // close socket 
    close(connfd);

    // free memory 
    free(info);
    return 0;
}

void init_details(Details* info) {

    info->username = NULL;
    info->password = NULL;
    info->folder = DEFAULT_FOLDER;
    info->command = NULL;
    info->server_name = NULL;
    info->message_num = NULL;
    // tcp used unless -t flag in commandline 
    info->TLS = USE_TCP;

}

