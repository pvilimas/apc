#ifndef APC_H
#define APC_H

#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "memory.h"

#define BN_NOFREE
#include "bignum.h"

// apc.h - arbitrary-precision calculator shell command

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

// initialize apc
void apc_init();

// evaluate the string and print the result
void apc_eval(const char* str);

// start repl mode ("q" to exit)
void apc_start_repl();

// apc uses two separate processes:
// main process - start child, wait, print result, repeat in repl mode
// child process - parse and evaluate expr, write error code, exit

// write error code to shared memory, exit from child proc with exit_code
void apc_return(ErrorCode exit_code);

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
    V_NUMBER
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        Bignum number;
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
    T_BASE, // _
    T_PLUS, // +
    T_MINUS, // -
    T_STAR, // *
    T_SLASH // /
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

void expr_print(const Expr* e);

// runtime

typedef struct {

    // shared memory
    ErrorCode* error_code;

    // runtime data

    // list of unary operators
    UnopData* unop_data;
    size_t n_unops;

    // list of binary operators
    BinopData* binop_data;
    size_t n_binops;

    // parser state

    // user input string currently being parsed
    stringview current_input;

    // used for scan_next_token()
    size_t last_index;

    // scan_next_token() and parser_next_token() write to this
    Token current_token;

    // list of tokens from the input string
    Token* tokens;
    size_t n_tokens;

    // used for parser_next_token()
    size_t token_index;

} Runtime;
extern Runtime runtime;

// parsing

// reads from runtime.current_input
// puts next token in runtime.current_token
// returns false if no more tokens
bool scan_next_token();

// read from the list of tokens and "consume" one
// increments runtime.token_index
// puts next token in runtime.current_token
// returns false if no more tokens
bool parser_next_token();

// optionally consumes a token
// returns true and gets next token if it matches the current one
// if optional_out is not NULL, runtime.current_token is also copied there
bool parser_accept(TokenType ttype);

// required, consumes a token
// apc_return(E_PARSE_ERROR) if not matched
// if optional_out is not NULL, runtime.current_token is also copied there
void parser_expect(TokenType ttype);

// does not consume a token
// loads the token after the current one into out
// returns false if not matched or the current one is the end
bool parser_lookahead(Token* out);

/*

numlit => \d+
    | \d+ "_" \d+

factor => numlit
    | "+" factor
    | "-" factor
    | "(" expr ")"

term => factor
    | factor "*" factor

expr => term
    | term "+" term
    | term "-" term

*/

Expr* consume_numlit();
Expr* consume_factor();
Expr* consume_term();
Expr* consume_expr();

// these constructors never fail or consume any tokens

// if opt_base is NULL it is ignored
Expr* build_expr_num(Token num, const Token* opt_base);
Expr* build_expr_unop(Token op, Expr* arg);
Expr* build_expr_binop(Token op, Expr* arg0, Expr* arg1);

// evaluating

Value eval_expr(const Expr* e);

// builtins.c

// unary operators
Value UnopFn_Plus(Value a0); // +a0
Value UnopFn_Minus(Value a0); // -a0

// binary operators
Value BinopFn_Add(Value a0, Value a1); // a0 - a1
Value BinopFn_Sub(Value a0, Value a1); // a0 + a1
Value BinopFn_Mul(Value a0, Value a1); // a0 * a1
Value BinopFn_Div(Value a0, Value a1); // a0 / a1

// utils.c

bool char_in_string(char c, const char* str);

// only handles () rn, []{} are ignored
bool parens_are_balanced(stringview sv);

void ctrl_c_signal_handler(int s);

#endif // APC_H
