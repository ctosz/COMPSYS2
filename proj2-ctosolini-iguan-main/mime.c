#include "mime.h"

void decode_mime(int connfd, char *message_num, int *tag_num)
{

    char *boundary_token = match_headers(connfd, message_num, tag_num);

    int len_param = strlen(boundary_token);
    // put into array
    char boundary_param[len_param];
    strncpy(boundary_param, boundary_token, len_param);
    boundary_param[len_param] = '\0';
    

    retrieve_contents_decoded(connfd, message_num, tag_num, boundary_param);
}

// check whether mime version and content type headers are correct
char *match_headers(int connfd, char *message_num, int *tag_num)
{

    (*tag_num)++;

    // Get MIME-Version header
    int command_size = snprintf(NULL, 0, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (MIME-Version)] \r\n", *tag_num, message_num);

    char mime_version_request[command_size];
    sprintf(mime_version_request, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (MIME-Version)] \r\n", *tag_num, message_num);

    ssize_t wstat = write(connfd, mime_version_request, strlen(mime_version_request));

    if (wstat == -1)
    {
        fprintf(stderr, "Error writing to server: getting MIME version\n");
        exit(IMAP_ERROR);
    }

    char mime_version_response[BUFFERLEN_READ];

    int rstat = read(connfd, mime_version_response, BUFFERLEN_READ);
    if (rstat < 0)
    {
        printf("Error reading server message: getting MIME version\n");
        exit(IMAP_ERROR);
    }
    mime_version_response[rstat] = '\0';

    // Check contents
    // Value of 1.0, no comment strings
    /* a formal BNF is given for the content of the MIME-Version field:
    version := "MIME-Version" ":" 1*DIGIT "." 1*DIGIT */

    char* mime_vers = strtok(mime_version_response, "\n");
    mime_vers = strtok(NULL, "\n");
    mime_vers += strlen(MIME_VERSION);

    if (strncasecmp(mime_vers, CORRECT_MIME_VERSION, strlen(CORRECT_MIME_VERSION)) != 0)
    {
        // Not the correct version.
        fprintf(stderr, "Error decoding MIME: incorrect version\n"); // exit with status 4
        exit(PARSE_FAILURE_ERROR);
    }
    // else {
    //     printf("yay\n");
    // }

    // Repeat for Content-Type header
    (*tag_num)++;

    command_size = snprintf(NULL, 0, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (Content-Type)] \r\n", *tag_num, message_num);

    char content_type_request[command_size];
    sprintf(content_type_request, "A%d FETCH %s BODY.PEEK[HEADER.FIELDS (Content-Type)] \r\n", *tag_num, message_num);

    wstat = write(connfd, content_type_request, strlen(content_type_request));

    if (wstat == -1)
    {
        fprintf(stderr, "Error writing to server: getting Content Type\n");
        exit(IMAP_ERROR);
    }

    char content_type_response[BUFFERLEN_READ];
    rstat = read(connfd, content_type_response, BUFFERLEN_READ);
    if (rstat < 0)
    {
        printf("Error reading server message: getting Content Type\n");
        exit(IMAP_ERROR);
    }
    content_type_response[rstat] = '\0';

    // Check contents
    // 1. Media Type
    char *media_type = strtok(content_type_response, "\n");
    media_type = strtok(NULL, "\n");

    media_type += strlen(CONTENT_TYPE);

    if (strncasecmp(media_type, CORRECT_MEDIA_TYPE, strlen(CORRECT_MEDIA_TYPE)) != 0)
    {
        // Not the correct version.
        fprintf(stderr, "Error decoding MIME: incorrect media type\n"); // exit with status 4
        exit(PARSE_FAILURE_ERROR);
    }
    // else
    // {
    //     printf("yay\n");
    // }

    // 2. boundary parameter
    //check if the boundary parameter exists, then store the value
    
    char *boundary_token;
    char *temp = strtok(NULL, "\n");

    if (strncasecmp(temp, " boundary=\"", strlen(" boundary=\"")) == 0) {
        // quote present so remove them
        boundary_token = remove_quotes(temp);
    }
    
    else {
        boundary_token = temp + strlen(MIME_TOKEN);
        boundary_token[strlen(boundary_token) - 1] = '\0';
    }

    if (strncasecmp(temp, MIME_TOKEN, strlen(MIME_TOKEN)) != 0)
    {
        // Not the correct version.
        fprintf(stderr, "Error decoding MIME: boundary parameter missing\n"); // exit with status 4
        exit(PARSE_FAILURE_ERROR);
    } 
    // else
    // {
    //     printf("yay\n");
    // }
    return boundary_token;
}

char* remove_quotes(char* str)
{   
    // find first double quote
    char* start = strchr(str, DOUBLE_QUOTE);
    start++; // skip it
    char* end = strrchr(str, DOUBLE_QUOTE);
    size_t len = end - start;
    
    char* content = malloc(len + 1);
    if (!content) {
        perror("Failed to allocate memory: contents removing quotes");
        exit(PARSE_FAILURE_ERROR);
    }

    strncpy(content, start, len);
    content[len] = '\0';
    return content;
}

void retrieve_contents_decoded(int connfd, char *message_num, int *tag_num, char *boundary_param)
{

    (*tag_num)++;
    size_t command_size;

    command_size = snprintf(NULL, 0, "A%d FETCH %s %s\r\n", *tag_num, message_num, MODE);
    char fetch_command[command_size];
    sprintf(fetch_command, "A%d FETCH %s %s\r\n", *tag_num, message_num, MODE);
    int wstat = write(connfd, fetch_command, strlen(fetch_command));
    if (wstat == -1)
    {
        fprintf(stderr, "Error writing to server: retrieving MIME-encoded email\n");
        exit(PARSE_FAILURE_ERROR);
    }

    // get full email content
    char *email_content = NULL;
    receive_email_content(connfd, &email_content);

    // concat "--" and boundary param val as delimiters for email body according to spec

    // CRLF--boundary-parameter-valueCRLF
    char *start_token = (char *)calloc(strlen(boundary_param) + 8, sizeof(char));
    strcat(start_token, CRLF);
    strcat(start_token, "--");
    strcat(start_token, boundary_param);
    strcat(start_token, CRLF);

    // CRLF--boundary-parameter-value--
    char *end_token = (char *)calloc(strlen(boundary_param) + 8, sizeof(char));
    strcat(end_token, CRLF);
    strcat(end_token, "--");
    strcat(end_token, boundary_param);
    strcat(end_token, "--");

    // find correct section of email using concat from above
    char *start_text = strstr(email_content, start_token);
    char *end_text = strstr(start_text, end_token);

    if (start_text == NULL || end_text == NULL)
    {
        fprintf(stderr, "Error: no MIME mail contents to decode\n");
        exit(PARSE_FAILURE_ERROR);
    }

    int section_length = end_text - start_text;
    char *first_part_start = (char *)malloc(section_length + 1); // +1 for null terminator
    strncpy(first_part_start, start_text, section_length);
    first_part_start[section_length] = '\0';

    first_part_start += strlen(start_token);
    char *first_part_end = strstr(first_part_start, start_token);

    if (first_part_start == NULL || first_part_end == NULL)
    {
        fprintf(stderr, "Fetch response parse error\n");
        exit(PARSE_FAILURE_ERROR);
    }

    int first_part_length = first_part_end - first_part_start;

    char *first_part = (char *)malloc(first_part_length + 1);
    strncpy(first_part, first_part_start, first_part_length);
    first_part[first_part_length] = '\0'; // null terminate 


    unfold_headers(first_part);

    int seen_headers = 0;
    // process the email body line by line
    char* line = strtok(first_part, "\n");
    check_param(line, &seen_headers);

    line = strtok(NULL, "\n");
    check_param(line, &seen_headers);

    line = strtok(NULL, "\n");
    check_param(line, &seen_headers);

    line = strtok(NULL, "\n");
    check_param(line, &seen_headers);

    // if the correct headers have been seen, seen_headers will == 2. if not, exit
    // i.e., content type & transfer encoding
    if (seen_headers != NUM_HEADERS_TO_CHECK) {
        fprintf(stderr, "MIME Error: Header values incorrect or missing\n");
        exit(PARSE_FAILURE_ERROR);
    }

    while (line != NULL) {
        printf("%s\n", line); 
        line = strtok(NULL, "\n");
    }

    // Free malloc'd memory

    free(first_part);
    free(start_token);
    free(end_token);
    free(email_content);
    
}

// unfolding is "simply removing any CRLF that is immediately followed by WSP"
void unfold_headers(char* email_body) {
    char* read = email_body;
    char* write = email_body;
    int headers = 1;
    
    while(*read) {

        // check if we are at the end of the headers
        if (headers && *read == '\r' && *(read + 1) == '\n' && *(read + 2) == '\r' && *(read + 3) == '\n') {
                headers = 0;
        }

        // unfold header 
        if (headers && *read == '\r' && *(read + 1) == '\n' && (*(read + 2) == '\t' || isspace(*(read + 2)))){
            read += strlen(CRLF); // skip the CRLF
        }
        *write++ = *read++;
    }
    *write = '\0';
}

void check_param(char* line, int* seen_headers) {

    if ((line == NULL) || (strncasecmp(line, HEADERS_FIRST_LETTER, 1) != 0)) {
        // whether no line OR first letter of the line is not the start of a useful header 
        return;
    }
    if (strncasecmp(line, CTE_QUOTED_PRINCIPLE, NUM_CHARS_TO_COMP) == 0) {
        // this is the transfer encoding header
        if ((strncasecmp(line, CTE_QUOTED_PRINCIPLE, strlen(CTE_QUOTED_PRINCIPLE)) != 0) && (strncasecmp(line, CTE_7BIT, strlen(CTE_7BIT)) != 0) && (strncasecmp(line, CTE_8BIT, strlen(CTE_8BIT)) != 0))
        {
            // Not the correct version.
            fprintf(stderr, "Error decoding MIME: incorrect transfer encoding\n"); // exit with status 4
            exit(PARSE_FAILURE_ERROR);
        }
        else
        {
            *seen_headers += 1;
            // printf("yay\n");
        }
    }
    else {
        // content type 
        if (strncasecmp(line, CONTENT_TYPE_BODY, strlen(CONTENT_TYPE_BODY)) != 0)
        {
            // Not the correct version.
            fprintf(stderr, "Error decoding MIME: incorrect content type\n"); // exit with status 4
            exit(PARSE_FAILURE_ERROR);
        }
        else
        {
            *seen_headers += 1;
            // printf("yay\n");

        }
    }
}


