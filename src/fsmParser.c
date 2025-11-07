// #include <includeinclude/fsmParser.h>
#include <fsmParser.h>
#include <string.h>
#include <stdio.h>
#include<unistd.h>


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

void handle_method(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    printf("byte: %c \n", buffer[parser->currPosition] );
    if (buffer[parser->currPosition] == ' ')
    {
        int length = parser->currPosition - parser->startValuePosition;
        printf("ENCONTRAMOS espacio y length es %i - %i = %i \n", parser->currPosition, parser->startValuePosition, length);
        // todo: podria crear una funcion como de cancelacion, un callback, o algo asi, que lo que haga es cancelar este parsing y cancelarle todo al usuairo porque es invalida la request
        if (length > MAX_METHOD_SIZE)
        {
            printf("ERROR SE PASO EL SIZE del MEthod, muy raro");
            change_state(parser, ERROR);
            return;
        }
        printf("pasamos bien \n");
        // todo: crear tambien una funcion que me evite esto, solo seria ponerlo como update o algo para evitar copiar y tener esto, pasandole el field de METHOD o URI como parametro
        memcpy(parser->http_request->method, &buffer[parser->startValuePosition], length);
        printf("copiamos memoria \n");
        printf("http_request pointer: %p\n", (void*)parser->http_request);
        parser->http_request->method[length+1] = '\0';
        printf("acutlaizamos y guardmaos !");
        parser->currPosition++;
        parser->startValuePosition = parser->currPosition;

        printf("nuestor valor de method es: %s \n", parser->http_request->method);
        change_state(parser, READING_URI);
        return;
    }

    parser->currPosition++;
}

void handle_uri(http_parser_t *parser, char *buffer)
{
    // finished reading our METHOD we found a space
    if (buffer[parser->currPosition == ' '])
    {
        int length = parser->currPosition - parser->startValuePosition;
        if (length > MAX_URI_SIZE)
        {
            printf("ERROR SE PASO EL SIZE de la URI, muy raro");
            change_state(parser, ERROR);
            return;
        }
        memcpy(parser->http_request->URI, &buffer[parser->startValuePosition], length);
        parser->http_request->URI[length+1] = '\0';
        parser->currPosition++;
        parser->startValuePosition = parser->currPosition;
        change_state(parser, READING_VERSION);
        return;
    }

    parser->currPosition++;
}
// todo: change this, i think we need to use the complete buffer, if not we can not use the memcpy from our StartingValue to the currentValue
// todo: using a while loop and a swtich, will work. and will be reading until the current goes on the lenght, and thats it, that will be the limit

int parse_http_request(http_parser_t *parser, char *buffer, int readedBytesOnBuffer)
{
    while (parser->currPosition < readedBytesOnBuffer)
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
            // handle_version(parser, buffer);
            printf("POR AHORA Deberiamos tener METHOD %s: \n URI %s \n", parser->http_request->method, parser->http_request->URI);
            sleep(2);

            // case READING_HEADER:
            //     handle_header(parser, buffer);

            // case READING_BODY:
            //     handle_body(parser, buffer);

            break;
        case ERROR:    
            printf("error detectado vamos a hacer un break \n");
            handle_start(parser);
            return -1;
        }    
    }    

    return 0;
}    
