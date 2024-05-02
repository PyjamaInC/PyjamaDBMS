#include "ParserExport.h"
#include "Sql_enums.h"
#include "../core/create_statement.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

extern parse_rc_t create_validate();
extern create_stm_data cdata;
int main(int argc, char** argv){

    parse_init();

    while (true) {

        printf("PyjamaDB -> ");
        fgets((char *)lex_buffer, sizeof(lex_buffer), stdin);

        if (lex_buffer[0] == '\n'){
            lex_buffer[0] = 0;
            continue;
        }

        lex_set_scan_buffer(lex_buffer);

        token_code = cyylex();

        switch (token_code){

            case SQL_SELECT_T:
                break;
            case SQL_INSERT_T:
                break;
            case SQL_CREATE_T:
                yyrewind(1);
                err = create_validate();
                if (err == PARSE_SUCCESS){
                    printf("Valid Create Query!\n");
                    analyse_create_stm(&cdata);
                }
                destroy_cdata(&cdata);
                break;
            default:
                printf("Error: Unrecofnized Query Input!\n");
                break;
        }
        Parser_stack_reset();
    }
    return 0;
}