%{
#include <stdio.h>
#include "dmmr_parser.h"

extern struct dmmr_parser* current_parser;
%}

%option noyywrap

%%

"ports"          {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "PORTS", yytext);
    return TOKEN_PORTS;
}

"interface"      {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "INTERFACE", yytext);
    return TOKEN_INTERFACE;
}

[0-9]+           {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "NUMBER", yytext);
    return TOKEN_NUMBER;
}

[a-zA-Z0-9_.:-]+ {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "STRING", yytext);
    return TOKEN_STRING;
}

"-"              {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "DASH", yytext);
    return TOKEN_DASH;
}

","              {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "COMMA", yytext);
    return TOKEN_COMMA;
}

";"              {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "SEMICOLON", yytext);
    return TOKEN_SEMICOLON;
}

[ \t\n\r]+       ; // Ignorar espaços

.                {
    if (current_parser && current_parser->handle_token)
        current_parser->handle_token(current_parser, "SYMBOL", yytext);
    return TOKEN_SYMBOL;
}

%%

int yywrap(void) {
    return 1;
}
