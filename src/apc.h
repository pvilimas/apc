#ifndef APC_H
#define APC_H

#include "bignum.h"

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

// apc.h - arbitrary-precision calculator

typedef enum {

    // normal result - print and continue
    E_OK = 0,
    E_NAME_ERROR,
    E_VALUE_ERROR,
    E_PARSE_ERROR,

    E_BAD_ERROR_LEVEL,

    // internal error - exit immediately
    E_MEMORY_ERROR,
    E_PROCESS_ERROR,
    E_INTERNAL_ERROR

} ErrorCode;

// run once to parse argv[1] or a line from stdin
void apc_eval(const char* str);

// start repl from stdin+stdout
void apc_start_repl();

// apc uses two separate processes:
// parser process - start child, wait, print result, repeat in repl mode
// child process - do all the work, write error code, exit

// init shared memory, set up parser
void apc_init();

// write error code to shared memory, exit from child proc with exit_code
void apc_return(ErrorCode exit_code);

// print result of child process from shared memory
void apc_print_result();

// unmap shared memory, exit from main proc with exit_code
void apc_exit(int exit_code);

// internal

#define DEBUG_PRINT 1
#define BREAKPOINT()                \
    printf("breakpoint @ %s:%d\n",  \
    (strrchr((__FILE__), '/')       \
    ? strrchr((__FILE__), '/') + 1  \
    : (__FILE__)), __LINE__)

// terminal io stuff
#define TERM_BOLD_ON "\033[1m"
#define TERM_BOLD_OFF "\033[0m"

// string types

// does not own data
typedef struct {
    char* str;
    size_t len;
} stringview;

#define sv_from(cstr) \
    ((stringview){.str = (char*)(cstr), .len = strlen((cstr))})

#define sv_slice(cstr, start, end) \
    ((stringview){.str = ((char*)(cstr) + (start)), .len = (end) - (start)})

#define sv_arg(sv) \
    (sv).len, (sv).str

// owns the data
typedef struct {
    char* str;
    size_t len;
} stringbuffer;

#define sb_new() \
    ((stringbuffer){0})

stringbuffer sb_copy(const stringbuffer* sb);

#define sb_free(sb) \
    do { \
        free((sb)->str); \
        *(sb) = ((stringbuffer){0}); \
    } while(0)

#define sb_slice(cstr, start, end) \
    ((stringbuffer){.str = strndup((cstr) + (start), (end) - (start)), .len = (end) - (start)})

#define sb_append_char(sb, c) \
    do { \
        (sb)->len += 1; \
        (sb)->str = realloc((sb)->str, (sb)->len * sizeof(char)); \
        (sb)->str[(sb)->len - 1] = (c); \
    } while(0)

// value - final/immediate result of a program

struct Value;

typedef enum {
    V_NUMBER,
    V_LIST
} ValueType;

typedef struct {
    struct Value* values;
    size_t n_values;
} List;

typedef struct Value {
    ValueType type;
    union {
        Bignum number;
        List list;
    };
} Value;

// unary and binary operators
// all operators are loaded at runtime

typedef Value (*UnopFn)(Value);
typedef Value (*BinopFn)(Value, Value);

typedef struct {
    char name;
    UnopFn fn;
} UnopData;

typedef struct {
    char name;
    BinopFn fn;
} BinopData;

// search runtime opdata
// these return null if name is not found
UnopData* get_unop(char name);
BinopData* get_binop(char name);

// token

typedef enum {
    T_NONE,
    T_IDENT, // \w+
    T_NUMBER, // \d+
    T_COMMA, // ,
    T_OPEN, // (
    T_CLOSE, // )
    T_PLUS, // +
    T_MINUS, // -
    T_STAR // *
} TokenType;

typedef struct {
    TokenType type;
    stringview atom;
} Token;

// ast

typedef struct Expr Expr;

typedef enum {
    X_VALUE,
    X_UNOP,
    X_BINOP
} ExprType;

typedef struct {
    UnopData* data;
    Expr* arg;
} Unop;

typedef struct {
    BinopData* data;
    Expr* arg0;
    Expr* arg1;
} Binop;

struct Expr {
    ExprType type;
    union {
        Value value;
        Unop unop;
        Binop binop;
    };
};

#define expr_new() \
    (calloc(1, sizeof(Expr)))

// runtime

typedef struct {

    // shared memory
    ErrorCode* error_code;

    // list of unary operators, loaded at runtime
    UnopData* unop_data;
    size_t n_unops;

    // list of binary operators, loaded at runtime
    BinopData* binop_data;
    size_t n_binops;

    // used for parsing
    Token current_token;
    stringview current_input;
    size_t last_index;

} Runtime;
extern Runtime runtime;

// parsing

// reads from runtime.current_input
// puts next token in runtime.current_token
// returns false if no more tokens
bool get_next_token();

// returns true and gets next token if it matches the current one
bool accept_token(TokenType ttype);

// exit with E_PARSE_ERROR if not matched
void expect_token(TokenType ttype);

// parse a full expression
Expr* consume_expr();

Expr* consume_term();
Expr* consume_factor();

// these do not consume any tokens

Expr* build_expr_num(Token num);
Expr* build_expr_unop(Token op, Expr* arg);
Expr* build_expr_binop(Token op, Expr* arg0, Expr* arg1);

// evaluating

Value eval_expr(const Expr* e);

// builtins.c

// unary operators
Value UnopFn_Plus(Value a0); // +a0
Value UnopFn_Negate(Value a0); // -a0

// binary operators
Value BinopFn_Add(Value a0, Value a1); // a0 - a1
Value BinopFn_Sub(Value a0, Value a1); // a0 + a1
Value BinopFn_Mul(Value a0, Value a1); // a0 * a1

// utils.c

bool char_in_string(char c, const char* str);

// only handles () rn, []{} are ignored
bool parens_are_balanced(stringview sv);

#endif // APC_H
