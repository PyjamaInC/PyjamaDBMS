#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "ParserExport.h"
#include "Sql_enums.h"
#include "../core/create_statement.h"


/*

create_validate -> create table <identifier> (COLSLIST)
COLSLIST -> COL | COL, COLSLIST
COL -> <identifier> DTYPE | <identifier> DTYPE primary key
DTYPE -> varchar (<number> | int | double )


*/

create_stm_data cdata;
parse_rc_t DATA_TYPE(){

    parse_init();
    token_code = cyylex();

    switch(token_code){

        case SQL_INT:
        case SQL_DOUBLE:
            cdata.col_data[cdata.n_cols].dtype = (sql_dtpye) token_code;
            cdata.col_data[cdata.n_cols].dtype_len = sql_dtype_size((sql_dtpye)token_code);
            RETURN_PARSE_SUCCESS;
        case SQL_STRING:
            // varchar token has token_code = SQL_STRING
            // expected: varchar is followed by a "("
            token_code = cyylex();
            if (token_code != SQL_BRACKET_LEFT){
                err_logging(token_code, SQL_BRACKET_LEFT);
                RETURN_PARSE_ERROR;
            }

            // expected: varchar take an integer argument inside "()", hence a number: 32, 64, etc
            token_code = cyylex();
            if (token_code != SQL_INTEGER_VALUE){
                err_logging(token_code, SQL_INTEGER_VALUE);
                RETURN_PARSE_ERROR;
            }

            cdata.col_data[cdata.n_cols].dtype = (sql_dtpye) token_code;
            cdata.col_data[cdata.n_cols].dtype_len = atoi(lex_curr_token);

            token_code = cyylex();
            if (token_code != SQL_BRACKET_RIGHT){
                err_logging(token_code, SQL_BRACKET_RIGHT);
                RETURN_PARSE_ERROR;
            }
            RETURN_PARSE_SUCCESS;
            break;
        default:
            RETURN_PARSE_ERROR;
    }

}

parse_rc_t COLUMN(){
    
    parse_init();
    int initial_chkp;
    CHECKPOINT(initial_chkp);

    memset(&cdata.col_data[cdata.n_cols], 0, sizeof(cdata.col_data[0]));

    // COLUMN -> <identifier> DATA_TYPE primary key
    do
    {
        // expected: a column name
        token_code = cyylex();
        if (token_code != SQL_IDENTIFIER) break;

        strncpy(cdata.col_data[cdata.n_cols].column_name, lex_curr_token, COLUMN_NAME_MAX_SIZE);
        // expected: int, double, varchar etc
        err = DATA_TYPE();
        if (err == PARSE_ERR) break;

        // expected: primary key
        token_code = cyylex();
        if (token_code != SQL_PRIMARY_KEY) break;
        cdata.col_data[cdata.n_cols].is_primary_key = true;
        cdata.n_cols++;
        
        RETURN_PARSE_SUCCESS;

    } while (0);

    RESTORE_CHKP(initial_chkp);
    memset(&cdata.col_data[cdata.n_cols], 0, sizeof(cdata.col_data[0]));

    // COLUMN -> <identifier> DATA_TYPE 
    do {
        
        // expected: a column name
        token_code = cyylex();
        if (token_code != SQL_IDENTIFIER){
            err_logging(token_code, SQL_IDENTIFIER);
        }
        strncpy(cdata.col_data[cdata.n_cols].column_name, lex_curr_token, COLUMN_NAME_MAX_SIZE);

        // expected: a data type: int, double, varchar
        err = DATA_TYPE();
        if (err == PARSE_ERR){
            RETURN_PARSE_ERROR;
        }

        cdata.n_cols++;
        RETURN_PARSE_SUCCESS;
    } while (0);

    cdata.n_cols++;
    RETURN_PARSE_SUCCESS;
}

parse_rc_t COLUMNS_LIST(){

    parse_init();
    int initial_chkp;
    CHECKPOINT(initial_chkp);

    do {

        err = COLUMN();

        if (err == PARSE_ERR) break;

        token_code = cyylex();
        if (token_code != SQL_COMMA){
            cdata.n_cols--;
            break;
        }

        err = COLUMNS_LIST();
        if (err == PARSE_ERR) break;

        RETURN_PARSE_SUCCESS;

    } while (0);

    RESTORE_CHKP(initial_chkp);

    do {

        err = COLUMN();

        if (err == PARSE_ERR) RETURN_PARSE_ERROR;
        RETURN_PARSE_SUCCESS;

    } while(0);

    RETURN_PARSE_SUCCESS;

}

parse_rc_t create_validate(){

    parse_init();
    memset(&cdata, 0, sizeof(cdata));

    // expected token: create
    token_code = cyylex();
    assert(token_code == SQL_CREATE_T);

    // expected token: table
    token_code = cyylex();
    if (strcmp(lex_curr_token, "table")){

        printf("Error: Expected keyword \'table', but the current token = %s\n", lex_curr_token);
        RETURN_PARSE_ERROR;
    }

    // expected token: an idenfier: name of the table
    token_code = cyylex();
    if (token_code != SQL_IDENTIFIER){
        err_logging(token_code, SQL_IDENTIFIER);
        RETURN_PARSE_ERROR;
    }
    strncpy(cdata.table_name, lex_curr_token, TABLE_NAME_MAX_SIZE);

    // expected token: a left bracket "("
    token_code = cyylex();
    if (token_code != SQL_BRACKET_LEFT){
        err_logging(token_code, SQL_BRACKET_LEFT);
        RETURN_PARSE_ERROR;
    }

    err = COLUMNS_LIST();
    if (err == PARSE_ERR){
        RETURN_PARSE_ERROR;
    }

    // expected token: a right bracket ")"
    token_code = cyylex();
    if (token_code != SQL_BRACKET_RIGHT){
        err_logging(token_code, SQL_BRACKET_RIGHT);
        RETURN_PARSE_ERROR;
    }

    token_code = cyylex();
    if (token_code != PARSER_EOL){
        err_logging(token_code, PARSER_EOL);
        printf("Failed\n");
        RETURN_PARSE_ERROR;
    }
    RETURN_PARSE_SUCCESS;

}
