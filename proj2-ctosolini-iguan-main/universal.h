#ifndef UNIVERSAL_H
#define UNIVERSAL_H
// universal functions and definitions etc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>

// for sockets from lecture 
#include <netdb.h>
#include <unistd.h>

#define BUFFERLEN_READ 2048 // is there a better way to do this?
#define CRLF "\r\n"

#define COMMANDLINE_PARSING_ERROR 1
#define CONNECTION_INITIATION_ERROR 2
#define IMAP_ERROR 3
#define PARSE_FAILURE_ERROR 4
#define OTHER_ERROR 5

typedef struct Details {

    char* username;
    char* password;
    char* folder;
    char* message_num;
    int TLS; // true or false 
    char* command;
    char* server_name;

} Details;

#endif 