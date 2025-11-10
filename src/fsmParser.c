// #include <includeinclude/fsmParser.h>
#include <fsmParser.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void change_state(http_parser_t *parser, PARSER_STATES newState)
{
    parser->state = newState;
}

void parse_init(http_parser_t *parser, http_request_t *request)
{
    parser->http_request = request;
}

void handle_start(http_parser_t *parser)
{
    parser->currPosition = 0;
    parser->startValuePosition = 0;
    memset(parser->http_request, 0, sizeof(*parser->http_request));
    change_state(parser, READING_METHOD);
}

void saveRequestValue(char *field, char *buffer, int length, http_parser_t *parser)
{
    memcpy(field, &buffer[parser->startValuePosition], length);
    field[length] = '\0';
    parser->currPosition++;
    parser->startValuePosition = parser->currPosition;
}

void handle_method(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    printf("byte: %c \n", buffer[parser->currPosition]);
    int length = (parser->currPosition) - parser->startValuePosition;
    if (length > MAX_METHOD_SIZE)
    {
        printf("ERROR SE PASO EL SIZE del MEthod, muy raro");
        change_state(parser, ERROR);
        return;
    }

    if (buffer[parser->currPosition] == ' ')
    {
        // todo: podria crear una funcion como de cancelacion, un callback, o algo asi, que lo que haga es cancelar este parsing y cancelarle todo al usuairo porque es invalida la request
        saveRequestValue(parser->http_request->method, buffer, length, parser);
        printf("nuestor valor de method es: %s \n", parser->http_request->method);
        change_state(parser, READING_URI);
        return;
    }
    parser->currPosition++;
}

void handle_uri(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    int length = (parser->currPosition) - parser->startValuePosition;
    if (length > MAX_URI_SIZE)
    {
        printf("ERROR SE PASO EL SIZE de la URI, muy raro");
        change_state(parser, ERROR);
        return;
    }

    if (buffer[parser->currPosition] == ' ')
    {
        saveRequestValue(parser->http_request->URI, buffer, length, parser);
        printf("nuestor valor de URI es: %s \n", parser->http_request->URI);
        change_state(parser, READING_VERSION);
        return;
    }
    printf("continuamos -- \n");
    parser->currPosition++;
}

void handle_version(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    // int length = parser->currPosition - parser->startValuePosition;
    int length = (parser->currPosition - parser->startValuePosition - 1);

    if (length > MAX_URI_SIZE)
    {
        printf("ERROR SE PASO EL SIZE de la URI, muy raro");
        change_state(parser, ERROR);
        return;
    }

    if (buffer[parser->currPosition] == '\n' && buffer[parser->currPosition - 1] == '\r')
    {
        saveRequestValue(parser->http_request->version, buffer, length, parser);
        printf("nuestor valor de version es: %s \n", parser->http_request->version);
        parser->http_request->ContentLength = 7;
        change_state(parser, READING_HEADER);
        return;
    }
    parser->currPosition++;
}

int isEndOfHeaderList(int currPosition, char *buffer)
{
    if (buffer[currPosition] == '\n' && buffer[currPosition - 1] == '\r' && buffer[currPosition - 2] == '\n' && buffer[currPosition - 3] == '\r')
    {
        return 1;
    }
    return 0;
}

void handle_header_key(http_parser_t *parser, char *buffer)
{
    // determine at the start of the header reading, if we are currently on the last \r\n\r\n so we jump to the reading body directly. Here we should have already the saved
    //  or something the content length, its important to have that to know how many bytes are on the body and so on.
    if (isEndOfHeaderList(parser->currPosition, buffer) == 1)
    {
        if (parser->http_request->ContentLength == 0)
        {
            change_state(parser, PRINT_REQUEST);
            return;
        }
        // because we would be still on the last \n so we update to start on the first byte of the body
        parser->currPosition++;
        parser->startValuePosition = parser->currPosition;
        change_state(parser, READING_BODY);
        return;
    }

    int length = parser->currPosition - parser->startValuePosition;
    if (length > MAX_HEADER_SIZE)
    {
        printf("ERROR SE PASO EL SIZE de header, muy raro");
        change_state(parser, ERROR);
        return;
    }

    if (buffer[parser->currPosition] == ':')
    {
        int headCount = parser->http_request->headerCount;
        saveRequestValue(parser->http_request->headers[headCount][0], buffer, length, parser);
        change_state(parser, READING_HEADER_VALUE);
        return;
    }
    parser->currPosition++;
}

void handle_header_value(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    int length = parser->currPosition - parser->startValuePosition - 1;
    if (length > MAX_HEADER_SIZE)
    {
        printf("ERROR SE PASO EL SIZE de la URI, muy raro");
        change_state(parser, ERROR);
        return;
    }

    if (buffer[parser->startValuePosition] == ' ')
    {
        parser->startValuePosition++;
    }

    if (buffer[parser->currPosition] == '\n' && buffer[parser->currPosition - 1] == '\r')
    {
        int headCount = parser->http_request->headerCount;
        char *headerValue = parser->http_request->headers[headCount][1];
        saveRequestValue(headerValue, buffer, length, parser);
        if (strcmp(parser->http_request->headers[headCount][0], "Content-Length") == 0)
        {

            char *endPtr;
            long len = strtol(headerValue, &endPtr, 10);

            if (len < 0 || len > MAX_BODY_SIZE)
            {
                printf("ERROR parseando el numero");
                change_state(parser, ERROR);
                return;
            }

            if (parser == NULL)
            {
                printf("EL PARSER era NULL \n");
                change_state(parser, ERROR);
            }

            if (parser->http_request == NULL)
            {
                printf("ERROR es NULL ese http request \n");
                change_state(parser, ERROR);
            }
            parser->http_request->ContentLength = (int)len;
            parser->http_request->body = malloc(parser->http_request->ContentLength);
        }
        parser->http_request->headerCount++;
        change_state(parser, READING_HEADER);
        return;
    }
    parser->currPosition++;
}

void handle_body(http_parser_t *parser, char *buffer)
{
    int length = parser->http_request->ContentLength;
    saveRequestValue(parser->http_request->body, buffer, length, parser);
    change_state(parser, PRINT_REQUEST);
}

int parse_http_request(http_parser_t *parser, char *buffer, int readedBytesOnBuffer)
{
    while (parser->currPosition <= readedBytesOnBuffer)
    // printf("PARSING WHILE LOOP \n");
    {

        switch (parser->state)
        {
        case START_PARSING:
            handle_start(parser);
            break;
        case READING_METHOD:
            handle_method(parser, buffer);
            break;
        case READING_URI:
            handle_uri(parser, buffer);
            break;
        case READING_VERSION:
            handle_version(parser, buffer);
            break;
        case READING_HEADER:
            //  headers[i][0]  // → nombre del header (clave)
            // headers[i][1]  // → valor del header
            handle_header_key(parser, buffer);

            break;
        case READING_HEADER_VALUE:
            handle_header_value(parser, buffer);
            break;
        case READING_BODY:
            handle_body_easy(parser, buffer);

            break;
        case ERROR:
            printf("error detectado vamos a hacer un break \n");
            handle_start(parser);
            return -1;
        case PRINT_REQUEST:
            printf("---- Finished Reading Request -- \n");
            printf("METHOD: %s \n URI: %s \n", parser->http_request->method, parser->http_request->URI);
            printf("URI: %s  \n", parser->http_request->URI);
            printf("Los headers son: \n");

            for (int i = 0; i < parser->http_request->headerCount; i++)
            {
                printf("HEADER: %s   VALUE: %s  \n", parser->http_request->headers[i][0], parser->http_request->headers[i][1]);
            }

            if (parser->http_request->ContentLength == 0){
                printf("BODY: Vacio \n");
                return 1;
            }
            printf("Body: %s \n", parser->http_request->body);
            return 1;
            break;
        }
    }

    return 0;
}
