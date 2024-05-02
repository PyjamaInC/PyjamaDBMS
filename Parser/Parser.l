%{

#include "ParserExport.h"
#include "Sql_enums.h"
// #include "MexprTree.h"
char lex_buffer[MAX_STRING_SIZE] = {0};
char* curr_ptr = lex_buffer;
stack_t stack = {-1, {0, 0, 0}};
char* lex_curr_token = NULL;
int lex_curr_token_len = 0;


void lex_push(lex_data_t lex_data){
    assert(stack.sp < MAX_STACK_SIZE - 1);
    stack.data[++stack.sp] = lex_data;
    lex_curr_token = (char*) lex_data.token_val;
    lex_curr_token_len = lex_data.token_leng;
};

lex_data_t lex_pop(){
    assert(stack.sp > -1);
    lex_data_t ret_data = stack.data[stack.sp--];

    if (stack.sp > -1){
        lex_data_t stck_top_lexd = stack.data[stack.sp];
        lex_curr_token = (char*) stck_top_lexd.token_val;
        lex_curr_token_len = stck_top_lexd.token_leng;
    } else {
        lex_curr_token = NULL;
        lex_curr_token_len = 0;
    };  
    return ret_data;
};

void process_white_space(int n){
    lex_data_t lex_data;
    curr_ptr += n;
    lex_data.token_code = PARSER_WHITE_SPACE;
    lex_data.token_leng = n;
    lex_data.token_val = NULL;
    lex_push(lex_data);
}

int cyylex(){
    int token_code = yylex();
    lex_data_t lex_data;
    lex_data.token_code = token_code;
    lex_data.token_leng = yyleng;
        
    curr_ptr += lex_data.token_leng;
    lex_data.token_val = (char*) calloc(1, yyleng+1);
    strncpy(lex_data.token_val, yytext, yyleng+1);

    lex_push(lex_data);

    return token_code;
}

void yyrewind(int n){

    if (n <= 0) return;
    if (curr_ptr == lex_buffer) return;

    int data_len = 0;
    lex_data_t lex_data;

    while (n){
        lex_data = lex_pop();
        data_len += lex_data.token_leng;

        if (lex_data.token_code == PARSER_WHITE_SPACE){
            continue;
        }

        n--;
        lex_data.token_code = 0;
        lex_data.token_leng = 0;
        
        if (lex_data.token_val){
            free(lex_data.token_val);
            lex_data.token_val = NULL;
        }
    }
    curr_ptr -= data_len;
    yy_scan_string(curr_ptr);
}

void RESTORE_CHKP(int a){
    lex_data_t lex_data;
    int data_len = 0;

    while (stack.sp > a){

        lex_data = lex_pop();
        data_len += lex_data.token_leng;

        if (lex_data.token_val){
            free(lex_data.token_val);
            lex_data.token_val = NULL;
        }
    }
    curr_ptr -= data_len;
    yy_scan_string(curr_ptr);
}

int peek_next_token(){
    int curr_sp = stack.sp;

    int next_token_code = cyylex();

    yyrewind(2);

    stack.sp = curr_sp;

    return next_token_code;

}

void Parser_stack_reset(){
    int i;
    lex_data_t *lex_data;
    
    for (i = 0; i < stack.sp; i++){
        lex_data = &stack.data[i];
        lex_data->token_code = 0;
        lex_data->token_leng = 0;
        if (lex_data->token_val){
            free(lex_data->token_val);
            lex_data->token_val = NULL;
        }
    }
    stack.sp = -1;
    curr_ptr = lex_buffer;
    lex_curr_token = NULL;
    lex_curr_token_len = 0;
}

void lex_set_scan_buffer(const char* buffer){
    yy_scan_string(buffer);
}

void print_stack() {
    if (stack.sp == -1){
        printf("Stack is empty.\n");
    }

    printf("Stack contents from top to bottom:\n");
    for (int i = stack.sp; i >= 0; i--){
        printf("Token code: %d, Token length: %d", stack.data[i].token_code, stack.data[i].token_leng);
        if (stack.data[i].token_val != NULL && stack.data[i].token_code != PARSER_EOL){
            printf(", Token value: %s\n", stack.data[i].token_val);
        } else if (stack.data[i].token_code == PARSER_EOL) {
            printf(", Token value: EOL\n");
        } else {
            printf(", Token value: NULL\n");
        }
    }
}

void print_current_token_info(int token_code) {
    // Print token information
    printf("Token Code: %d, Token Value: %s\n", token_code, yytext);
}

void print_currtoken_formatted(int token_code){
    printf("    ");
    print_current_token_info(token_code);
}

%}

%%

"select" {
    return SQL_SELECT_T;
}

"create" {
    return SQL_CREATE_T;
}

"insert into" {
    return SQL_INSERT_T;
}

"delete" {
    return SQL_DELETE_T;
}

"update" {
    return SQL_UPDATE_T;
}

"drop" {
    return SQL_DROP_TABLE_T;
}

"varchar" {
    return SQL_STRING;
}

"int" {
    return SQL_INT;
}

decimal|double {
    return SQL_DOUBLE;
}

"primary key" {
    return SQL_PRIMARY_KEY;
}

"not null" {
    return SQL_NOT_NULL;
}

"max" {
    return SQL_MAX;
}

"min" {
    return SQL_MIN;
}

"count" {
    return SQL_COUNT;
}

"avg" {
    return SQL_AVG;
}

"from" {
    return SQL_FROM;
}

"where" {
    return SQL_WHERE;
}

"group by" {
    return SQL_GROUP_BY;
}

"order by" {
    return SQL_ORDER_BY;
}

"having" {
    return SQL_HAVING;
}

"asc" {
    return SQL_ORDERBY_ASC;
}

"desc" {
    return SQL_ORDERBY_DESC;
}

"like" {
    return SQL_LIKE;
}

"in" {
    return SQL_IN;
}

"(" {
    return SQL_BRACKET_LEFT;
}

")" {
    return SQL_BRACKET_RIGHT;
}

"<" {
    return SQL_LESS_THAN;
}

"<=" {
    return SQL_LESS_OR_EQ;
}

">" {
    return SQL_GREATER_THAN;
}

">=" {
    return SQL_GREATER_OR_EQ;
}

"=" {
    return SQL_EQ;
}

"!=" {
    return SQL_NOT;
}

"and" {
    return SQL_AND;
}

"or" {
    return SQL_OR;
}

"," {
    return SQL_COMMA;
}

"*" {
    return MATH_MUL;
}

"/" {
    return MATH_DIV;
}

"+" {
    return MATH_PLUS;
}

"-" {
    return MATH_MINUS;
}

"sqr" {
    return MATH_SQR;
}

"sqrt" {
    return MATH_SQRT;
}

"hmax" {
    return MATH_MAX;
}

"hmin" {
    return MATH_MIN;
}

"sin" {
    return MATH_SIN;
}

"cos" {
    return MATH_COS;
}

"pow" {
    return MATH_POW;
}

"?" {
    return SQL_QUOTATION_MARK;
}

\n {
    return PARSER_EOL;
}

"\\\n" {

}

"\\q" {
    return PARSER_QUIT;
}


0|-?[1-9][0-9]* {
    return SQL_INTEGER_VALUE;
}

-?[0-9]*\.[0-9]+ {
    return SQL_STRING_VALUE;
}

[a-zA-Z0-9_]+ {
    return SQL_IDENTIFIER;
}

[a-zA-Z0-9_]*\.[a-zA-Z0-9_]+ {
    return SQL_IDENTIFIER_IDENTIFIER;
}

'[^']*' {
    return SQL_STRING_VALUE;
}

\"[^"]*\" {
    return SQL_STRING_VALUE;
}

[ ] {
    process_white_space(1);
}

[\t] {
    process_white_space(4);
}

. {
    // ignore
}

%% 





