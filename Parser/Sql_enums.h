#ifndef SQL_ENUMS_H
#define SQL_ENUMS_H

typedef enum _sql_generics_ {

    SQL_IDENTIFIER,
    SQL_IDENTIFIER_IDENTIFIER,
    SQL_QUERY_TYPE,
    SQL_AGGREGATE_FN,
    SQL_KEYWORD,
    SQL_OPERATOR,
    SQL_DTYPE,
    SQL_GENERICS_MAX

} sql_generics_type;

typedef enum _sql_query_type_ {

    SQL_SELECT_T = SQL_GENERICS_MAX + 1,
    SQL_INSERT_T,
    SQL_UPDATE_T,
    SQL_DELETE_T,
    SQL_CREATE_T,
    SQL_DROP_TABLE_T,
    SQL_UNSUPPORTED_T

} sql_query_type;


typedef enum _sql_aggregate_fn_ {

    SQL_SUM = SQL_UNSUPPORTED_T + 1,
    SQL_MIN,
    SQL_MAX,
    SQL_COUNT,
    SQL_AVG,
    SQL_AGG_FN_NONE

} sql_aggregate_fn;

typedef enum _sql_keywords_ {

    SQL_FROM = SQL_AGG_FN_NONE + 1,
    SQL_WHERE,
    SQL_GROUP_BY,
    SQL_HAVING,
    SQL_ORDER_BY,
    SQL_LIMIT,
    SQL_PRIMARY_KEY,
    SQL_NOT_NULL,
    SQL_AS,
    SQL_SELECT,
    SQL_KEYWORD_MAX

} sql_keywords;

typedef enum _sql_operators_ {

    SQL_LESS_THAN,
    SQL_LESS_OR_EQ,
    SQL_GREATER_OR_EQ,
    SQL_GREATER_THAN,
    SQL_AND,
    SQL_OR,
    SQL_EQ,
    SQL_NOT_EQ,
    SQL_NOT,
    SQL_IN,
    SQL_LIKE,
    SQL_BETWEEN,
    SQL_OPERATOR_MAX

} sql_operator;

typedef enum _sql_dtype_ {

    SQL_DTYPE_FIRST = SQL_OPERATOR_MAX + 1,
    SQL_STRING,
    SQL_INT,
    SQL_DOUBLE,
    SQL_BOOL,
    SQL_DTYPE_MAX,

} sql_dtpye;

typedef enum _sql_dtype_attr_ {

    SQL_DTYPE_LEN = SQL_DTYPE_MAX + 1,
    SQL_DTYPE_ATTR_MAX

} sql_dtype_attr;

typedef enum _sql_rhs_ {

    SQL_INTEGER_VALUE = SQL_DTYPE_ATTR_MAX + 1,
    SQL_STRING_VALUE,
    SQL_DOUBLE_VALUE,
    SQL_RHS_MAX,

} sql_rhs;

typedef enum _math_fns_ {

    MATH_MAX = SQL_RHS_MAX + 1,
    MATH_MIN,
    MATH_PLUS,
    MATH_MINUS,
    MATH_MUL,
    MATH_DIV,
    MATH_SQRT,
    MATH_SQR,
    MATH_SIN,
    MATH_COS,
    MATH_POW,
    MATH_MOD,
    MATH_FN_MAX,

} math_fns;

typedef enum _sql_order_ {

    SQL_ORDERBY_ASC = MATH_FN_MAX + 1,
    SQL_ORDERBY_DESC,
    SQL_ORDERBY_MAX,

} sql_order;

typedef enum _sql_misc_ {

    SQL_COMMA = SQL_ORDERBY_MAX + 1,
    SQL_BRACKET_LEFT,
    SQL_BRACKET_RIGHT,
    SQL_QUOTATION_MARK,

} sql_misc;

static inline int sql_valid_dtype(int dtype) {

    switch (dtype) {

        case SQL_STRING:
        case SQL_DOUBLE:
        case SQL_INT:
            return 1;
    }
    return 0;

}

static inline const char *dtype_to_str(sql_dtpye dtype) {

    switch (dtype) {

        case SQL_STRING:
            return "SQL_STRING";
        case SQL_DOUBLE:
            return "SQL_DOUBLE";
        case SQL_INT:
            return "SQL_INT";
    }
    return 0;

}

static inline int sql_dtype_size(sql_dtpye dtype){

    switch (dtype) {

        case SQL_STRING:
            return 1;
        case SQL_DOUBLE:
            return sizeof(double);
        case SQL_INT:
            return 4;
    }
    return 0;

}

static inline const char *sql_fn_tostr(sql_aggregate_fn agg_func){

    switch (agg_func){

        case SQL_SUM:
            return "sum";
        case SQL_MIN:
            return "min";
        case SQL_MAX:
            return "max";
        case SQL_COUNT:
            return "count";
        case SQL_AVG:
            return "avg";

    }
    return 0;
}


#endif // SQL_ENUMS_H