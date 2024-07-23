#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "ParserExport.h"
#include "Sql_enums.h"

static parse_rc_t VALUE(){

    parse_init();

    token_code = cyylex();

    switch (token_code){

        case SQL_INTEGER_VALUE:
        case SQL_DOUBLE_VALUE:
        case SQL_STRING_VALUE:
            RETURN_PARSE_SUCCESS;

        default:
            RETURN_PARSE_ERROR;

    }
    RETURN_PARSE_ERROR;

}

static parse_rc_t VALUES(){

    parse_init();

    int initial_chkp;

    do {

        err = VALUE();
        if(err == PARSE_ERR) break;

        token_code = cyylex();

        if (token_code != SQL_COMMA) break;

        err = VALUES();
        if (err =- PARSE_ERR) break;

        RETURN_PARSE_SUCCESS;

    } while(0);
    RESTORE_CHKP(initial_chkp);

    do {

        err = VALUE();

        if (err == PARSE_ERR) RETURN_PARSE_ERROR;

        RETURN_PARSE_SUCCESS;

    }while(0);

    RETURN_PARSE_SUCCESS;
}

// insert_validate() -> insert into IDENTIFIER values (VALUES) 
parse_rc_t insert_validate(){

    parse_init();

    token_code = cyylex();

    assert(token_code == SQL_INSERT_T);

    token_code = cyylex();

    if (token_code != SQL_IDENTIFIER){
        err_logging(token_code, SQL_IDENTIFIER);
        RETURN_PARSE_ERROR;
    }

    token_code = cyylex();

    if (strcmp(lex_curr_token, "values")){
        printf("Error: Expected keyword\'values\' is missing = %s\n", lex_curr_token);
        RETURN_PARSE_ERROR;
    }

    token_code = cyylex();

    if (token_code != SQL_BRACKET_LEFT){
        err_logging(token_code, SQL_BRACKET_LEFT);
        RETURN_PARSE_ERROR;
    }

    err = VALUES();

    if (err == PARSE_ERR) RETURN_PARSE_ERROR;

    token_code = cyylex();

    if (token_code != SQL_BRACKET_RIGHT){
        err_logging(token_code, SQL_BRACKET_RIGHT);
        RETURN_PARSE_ERROR;
    }

    if (token_code != PARSER_EOL){
        err_logging(token_code, PARSER_EOL);
        RETURN_PARSE_ERROR;
    }

}