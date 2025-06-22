%option noyywrap
%option nounput
%option noinput
%option yylineno

%{
#include <stdio.h>
#include <console.h>
#include <l_tokens.h>

// Usar a função de tratamento interno
extern void handle_token(const char*, const char*);
#define SEND_TOKEN(type, val) handle_token(type, val)
%}

DIGIT   [0-9]
LETTER  [a-zA-Z]
IDENT   {LETTER}({LETTER}|{DIGIT}|_)*
WS      [ \t]
NEWLINE \n

%%

"SESSION_SIZE"                  { SEND_TOKEN("KEYWORD", "SESSION_SIZE"); return TOKEN_IDENT; }
"SLEEP_TIME"                    { SEND_TOKEN("KEYWORD", "SLEEP_TIME"); return TOKEN_IDENT; }
"CIRCLE_BUFFER_SIZE"            { SEND_TOKEN("KEYWORD", "CIRCLE_BUFFER_SIZE"); return TOKEN_IDENT; }
"MAX_PORTS"                     { SEND_TOKEN("KEYWORD", "MAX_PORTS"); return TOKEN_IDENT; }
"REAL_TIME_DEAD_LINE"           { SEND_TOKEN("KEYWORD", "REAL_TIME_DEAD_LINE"); return TOKEN_IDENT; }
"REAL_TIME_USER_DEFINED"        { SEND_TOKEN("KEYWORD", "REAL_TIME_USER_DEFINED"); return TOKEN_IDENT; }
"TRUNK_ACCEPT_URI"              { SEND_TOKEN("KEYWORD", "TRUNK_ACCEPT_URI"); return TOKEN_IDENT; }
"TRUNK_DISPATCH_URI"            { SEND_TOKEN("KEYWORD", "TRUNK_DISPATCH_URI"); return TOKEN_IDENT; }
"SCHEDULER_PREEMPTIVE_DEADLINE" { SEND_TOKEN("KEYWORD", "SCHEDULER_PREEMPTIVE_DEADLINE"); return TOKEN_IDENT; }
"RB_LOG_LEVEL"                  { SEND_TOKEN("KEYWORD", "RB_LOG_LEVEL"); return TOKEN_IDENT; }

{DIGIT}+ {
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

{IDENT} {
    SEND_TOKEN("IDENT", yytext);
    return TOKEN_IDENT;
}

\"([^\\\"]|\\.)*\" {
    SEND_TOKEN("STRING", yytext);
    return TOKEN_STRING;
}

{WS}+    { /* Ignorar espacos */ }
{NEWLINE} { /* Ignorar novas linhas */ }

. {
    SEND_TOKEN("INVALID", yytext);
    return TOKEN_ERROR;
}

%%
