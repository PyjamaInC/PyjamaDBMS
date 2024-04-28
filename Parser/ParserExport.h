#ifndef PARSER_EXPORT_H
#define PARSER_EXPORT_H

#include <assert.h>
// #include "MexprEnums.h"
#define PARSER_EOL 10000
#define PARSER_QUIT 10001
#define PARSER_WHITE_SPACE 10002

typedef enum parse_rc_ {
    PARSE_ERR,
    PARSE_SUCCESS,
} parse_rc_t;

typedef struct lex_data_ {
    int token_code;
    int token_leng;
    char* token_val;
} lex_data_t;

#define MAX_STACK_SIZE 512
#define MAX_STRING_SIZE 512

typedef struct stack_ {
    int sp;
    lex_data_t data[MAX_STACK_SIZE];
} stack_t;

extern "C" int yylex();
extern char lex_buffer[MAX_STRING_SIZE];
extern stack_t stack;
extern char* curr_ptr;
extern char* lex_curr_token;
extern int lex_curr_token_len;

#define CHECKPOINT(a) a = stack.sp
// #define RESTORE_CHKP(a) yyrewind(stack.sp - a)
#define RETURN_PARSE_ERROR {RESTORE_CHKP(_lchkp); return PARSE_ERR;}
#define RETURN_PARSE_SUCCESS return PARSE_SUCCESS

// static inline void parse_init() {
//     int token_code = 0;
//     int _lchkp = stack.sp;
//     parse_rc_t err = PARSE_SUCCESS;
// }

#define parse_init() \
    int token_code = 0; \
    int _lchkp = stack.sp; \
    parse_rc_t err = PARSE_SUCCESS;

#define err_logging(token_code, proper_code) \
    printf("%s(%d) : Token Obtained = %d (%s), expected token = (%d)\n", \
    __FUNCTION__, __LINE__, token_code, lex_curr_token, proper_code);

extern void lex_push(lex_data_t lex_data);
extern lex_data_t lex_pop();
extern void process_white_space(int n);
extern int cyylex();
extern void yyrewind(int n);
extern int peek_next_token();
extern int is_inequality_operator(int token_code);
extern void Parser_stack_reset();
// extern void parse_init();
extern void lex_set_scan_buffer(const char* buffer);
extern void print_current_token_info(int token_code);
extern void print_currtoken_formatted(int token_code);
extern void print_stack();
extern void RESTORE_CHKP(int a);

#endif