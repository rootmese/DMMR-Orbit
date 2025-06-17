%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dmmr_parser.h"

extern struct dmmr_parser* current_parser;

#define SEND_TOKEN(type, val) \
    do { \
        if (current_parser && current_parser->handle_token) \
            current_parser->handle_token(current_parser, type, val); \
    } while (0)
%}

%option noyywrap

%%

SCHEDULER_PREEMPTIVE_DEADLINE   { SEND_TOKEN("KEYWORD", "SCHEDULER_PREEMPTIVE_DEADLINE"); return TOKEN_IDENT; }
SLEEP_TIME                      { SEND_TOKEN("KEYWORD", "SLEEP_TIME"); return TOKEN_IDENT; }
SESSION_SIZE                    { SEND_TOKEN("KEYWORD", "SESSION_SIZE"); return TOKEN_IDENT; }
CIRCLE_BUFFER_SIZE              { SEND_TOKEN("KEYWORD", "CIRCLE_BUFFER_SIZE"); return TOKEN_IDENT; }
MAX_PORTS                       { SEND_TOKEN("KEYWORD", "MAX_PORTS"); return TOKEN_IDENT; }
REAL_TIME_DEAD_LINE             { SEND_TOKEN("KEYWORD", "REAL_TIME_DEAD_LINE"); return TOKEN_IDENT; }
REAL_TIME_USER_DEFINED          { SEND_TOKEN("KEYWORD", "REAL_TIME_USER_DEFINED"); return TOKEN_IDENT; }
TRUNK_ACCEPT_URI                { SEND_TOKEN("KEYWORD", "TRUNK_ACCEPT_URI"); return TOKEN_IDENT; }
TRUNK_DISPATCH_URI              { SEND_TOKEN("KEYWORD", "TRUNK_DISPATCH_URI"); return TOKEN_IDENT; }


[0-9]+ {
    SEND_TOKEN("NUMBER", yytext);
    return TOKEN_NUMBER;
}

"," {
    SEND_TOKEN("COMMA", yytext);
    return TOKEN_COMMA;
}

";" {
    SEND_TOKEN("SEMICOLON", yytext);
    return TOKEN_SEMICOLON;
}

"-" {
    SEND_TOKEN("DASH", yytext);
    return TOKEN_SYMBOL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    SEND_TOKEN("IDENT", yytext);
    return TOKEN_IDENT;
}

\"[^\"]*\" {
    SEND_TOKEN("STRING", yytext);
    return TOKEN_STRING;
}

[ \t\n\r]+  { /* skip */ }

. {
    SEND_TOKEN("SYMBOL", yytext);
    return TOKEN_SYMBOL;
}

%%

int yywrap(void) {
    return 1;
}
