#ifndef PARSER_H
#define PARSER_H

// #include <include/http.h>
#include <http.h>
typedef enum {
    START_PARSING,
    READING_METHOD,
    READING_URI,
    READING_VERSION,
    READING_HEADER,
    READING_HEADER_VALUE,
    READING_BODY,
    ERROR,
}PARSER_STATES;

typedef struct{
    int currPosition;
    int startValuePosition;
    PARSER_STATES state; 
    http_request_t *http_request;
}http_parser_t;





void parse_init(http_parser_t *parser, http_request_t *request);
int parser_consume_byte(http_parser_t *parser, char byte);
int parse_http_request(http_parser_t *parser, char *buffer, int nBytes);
#endif