#include "apc.h"

// global instance
Runtime runtime = {0};

void apc_init() {

    signal(SIGINT, ctrl_c_signal_handler);

    // init shared memory
    runtime.error_code = mmap(NULL, sizeof(ErrorCode),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANON,
        -1, 0);
    if (runtime.error_code == MAP_FAILED) {
        fputs(" = memory error\n", stdout);
        exit(E_MEMORY_ERROR);
    }

    // init opdata lookup tables
    runtime.n_unops = 2;
    runtime.unop_data = apc_malloc(runtime.n_unops * sizeof(UnopData));
    runtime.unop_data[0] = (UnopData){'+', UnopFn_Plus};
    runtime.unop_data[1] = (UnopData){'-', UnopFn_Minus};

    runtime.n_binops = 6;
    runtime.binop_data = apc_malloc(runtime.n_binops * sizeof(BinopData));
    runtime.binop_data[0] = (BinopData){'+', BinopFn_Add};
    runtime.binop_data[1] = (BinopData){'-', BinopFn_Sub};
    runtime.binop_data[2] = (BinopData){'*', BinopFn_Mul};
    runtime.binop_data[3] = (BinopData){'/', BinopFn_Div};
    runtime.binop_data[4] = (BinopData){'%', BinopFn_Mod};
    // explicit base _ is not an operator, handled in consume_numlit
    runtime.binop_data[5] = (BinopData){'#', BinopFn_BaseConv};

    // init bignum library stuff
    BN_CONFIG.no_free = BC_NF_ENABLED;
    BN_CONFIG.malloc_hook = apc_malloc;
    BN_CONFIG.realloc_hook = apc_realloc;
    BN_CONFIG.free_hook = apc_free;
}

void apc_exit(int exit_code) {
    munmap(runtime.error_code, sizeof(ErrorCode));
    exit(exit_code);
}

void apc_eval(const char* str) {

    int pid = fork();
    if (pid == 0) {
        // child

        // set error code to something bad in case child exits w/o apc_return
        *runtime.error_code = E_INTERNAL_ERROR;

        runtime.current_input = sv_from(str);

        if (!parens_are_balanced(runtime.current_input)) {
            apc_return(E_PARSE_ERROR);
        }

        // "" => parse error
        // this will never trigger in the repl, only from argv
        if (runtime.current_input.len == 0) {
            apc_return(E_PARSE_ERROR);
        }

        // init
        runtime.tokens = NULL;
        runtime.n_tokens = 0;
        runtime.last_index = 0;
        runtime.current_token = (Token){T_NONE};

        // read all tokens into a list
        while(scan_next_token()) {
            runtime.n_tokens += 1;
            runtime.tokens = apc_realloc(runtime.tokens,
                runtime.n_tokens * sizeof(Token));
            runtime.tokens[runtime.n_tokens-1] = runtime.current_token;
        }

        // parse
        runtime.token_index = 0;
        runtime.current_token = (Token){T_NONE};
        parser_next_token();

        Expr* e = consume_expr();

        // eval
        Value final_result = eval_expr(e);

        // print
        fputs(" = ", stdout);

        // always print explicit base if not base10
        // always print in uppercase for now to match python
        bn_print2(&final_result.number,
            final_result.number.base != BN_BASE_DEFAULT,
            BN_PRINT_UPPERCASE);

        // done
        *runtime.error_code = E_OK;
        apc_return(E_OK);

    } else if (pid > 0) {
        // parent
        wait(NULL);

        if (*runtime.error_code == E_OK) {
            // do nothing, child already printed result
        } else if (*runtime.error_code == E_NAME_ERROR) {
            fputs(" = name error", stdout);
        } else if (*runtime.error_code == E_VALUE_ERROR) {
            fputs(" = value error", stdout);
        } else if (*runtime.error_code == E_PARSE_ERROR) {
            fputs(" = syntax error", stdout);
        } else if (*runtime.error_code == E_MEMORY_ERROR) {
            fputs(" = memory error", stdout);
        } else if (*runtime.error_code == E_PROCESS_ERROR) {
            fputs(" = process error", stdout);
        } else if (*runtime.error_code == E_INTERNAL_ERROR) {
            fputs(" = internal error", stdout);
        } else {
            fputs(" = internal error", stdout);
        }
        fputc('\n', stdout);
    } else {
        // fork failed
        *runtime.error_code = E_PROCESS_ERROR;
        fputs(" = process error\n", stdout);
        apc_exit(E_PROCESS_ERROR);
    }
}

void apc_start_repl() {

    char* line = NULL;
    size_t len = 0;

    while (1) {

        fputs(" = ", stdout);
        // TODO write custom getline that uses apc_malloc to exit on error
        // or do specific things when EOF is read
        ssize_t nread = getline(&line, &len, stdin);

        if (nread == -1 || line == NULL) {
            // used pressed Ctrl+D
            fputs("^D\n", stdout);
            apc_exit(E_OK);
        }

        if (line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
            nread -= 1;
        }

        if (nread == 0) {
            continue; // ""
        }

        if (!strncmp(line, "q", len) || !strncmp(line, "quit", len)) {
            apc_exit(E_OK); // user exited repl
        }

        apc_eval(line);

        // exit on "bad" error types
        if (*runtime.error_code >= E_BAD_ERROR_LEVEL) {
            apc_exit(*runtime.error_code);
        }
    }

    apc_exit(E_INTERNAL_ERROR); // how did we get here
}

void apc_return(ErrorCode exit_code) {
    *runtime.error_code = exit_code;
    exit(exit_code);
}

// internal

stringbuffer sb_copy(const stringbuffer* sb) {
    stringbuffer sb2 = {
        .len = sb->len,
        .str = apc_malloc(sb->len * sizeof(char))
    };
    memcpy(sb2.str, sb->str, sb->len);
    sb2.str[sb->len] = '\0';

    return sb2;
}

UnopData* rt_get_unop(char name) {
    for (size_t i = 0; i < runtime.n_unops; i++) {
        if (name == runtime.unop_data[i].name) {
            return &runtime.unop_data[i];
        }
    }
    return NULL;
}

BinopData* rt_get_binop(char name) {
    for (size_t i = 0; i < runtime.n_binops; i++) {
        if (name == runtime.binop_data[i].name) {
            return &runtime.binop_data[i];
        }
    }
    return NULL;
}

void expr_print(const Expr* e) {
    if (e->type == X_VALUE) {
        fputs("Value{", stdout);
        bn_print2(&e->value.number, true, false);
        fputc('}', stdout);
    } else if (e->type == X_UNOP) {
        printf("Unop{%c, ", e->unop.data->name);
        expr_print(e->unop.arg);
        fputc('}', stdout);
    } else if (e->type == X_BINOP) {
        printf("Binop{%c, ", e->binop.data->name);
        expr_print(e->binop.arg0);
        fputs(", ", stdout);
        expr_print(e->binop.arg1);
        fputc('}', stdout);
    } else {
        printf("Expr{???}");
    }
}

bool scan_next_token() {
    const char* s = runtime.current_input.str;
    size_t l = runtime.current_input.len;

    if (s == NULL
    || l == 0
    || runtime.last_index >= l) {
        return false;
    }

    // skip leading spaces
    while (runtime.last_index < l
    && isspace(s[runtime.last_index])) {
        runtime.last_index += 1;
    }

    // "   \t  "
    if (runtime.last_index == l) {
        return false;
    }

    Token t = {
        .type = T_NONE,
        .atom = { .str = (char*)s + runtime.last_index, .len = 1 }
    };

    // consume a single character (an operator)

    char c = t.atom.str[0];
    if (c == ',') t.type = T_COMMA;
    else if (c == '(') t.type = T_OPEN;
    else if (c == ')') t.type = T_CLOSE;
    else if (c == '_') t.type = T_BASE;
    else if (c == '+') t.type = T_PLUS;
    else if (c == '-') t.type = T_MINUS;
    else if (c == '*') t.type = T_STAR;
    else if (c == '/') t.type = T_SLASH;
    else if (c == '%') t.type = T_PERCENT;
    else if (c == '#') t.type = T_CONV;

    if (t.type != T_NONE) {
        // advance ptr and write token
        runtime.last_index += t.atom.len;
        runtime.current_token = t;
        return true;
    }

    // consume a whole word, stopping at first space or operator

    t.type = T_NUMBER;
    while (runtime.last_index < l) {
        c = s[runtime.last_index];

        if (char_in_string(c, ",()_+-*/%#") || isspace(c)) {
            break;
        } else if (!isalnum(c)) {
            apc_return(E_PARSE_ERROR);
        }

        t.atom.len += 1;
        runtime.last_index += 1;
    }

    // backtrack by 1, to handle this character (either a single char operator
    // or whitespace) next iteration
    t.atom.len -= 1;

    // if there was a-zA-Z in the word and it doesn't have explicit base,
    // it's an identifier

    // advance ptr and write token
    runtime.current_token = t;
    return true;
}

bool parser_next_token() {
    if (runtime.token_index >= runtime.n_tokens) {
        return false;
    }

    runtime.current_token = runtime.tokens[runtime.token_index];
    runtime.token_index += 1;
    return true;
}

bool parser_accept(TokenType ttype) {
    if (runtime.current_token.type == ttype) {
        parser_next_token();
        return true;
    }
    return false;
}

void parser_expect(TokenType ttype) {
    if (!parser_accept(ttype)) {
        apc_return(E_PARSE_ERROR);
    }
}

bool parser_lookahead(Token* out) {
    if (runtime.token_index + 1 >= runtime.n_tokens) {
        return false;
    }

    *out = runtime.tokens[runtime.token_index + 1];
    return true;
}

Expr* consume_numlit() {

    Token t_num;
    Token t_base;
    Token t_base_10 = { .type=T_NUMBER, .atom=sv_from("10") };

    // bignum value
    t_num = runtime.current_token;
    if (parser_accept(T_NUMBER)) {

        // bignum with explicit base
        if (parser_accept(T_BASE)) {
            t_base = runtime.current_token;
            parser_expect(T_NUMBER);
            return build_expr_num(t_num, &t_base);
        }

        // bignum without base - default to 10
        return build_expr_num(t_num, &t_base_10);
    }

    apc_return(E_PARSE_ERROR);
    // unreachable
    return NULL;
}

Expr* consume_factor() {

    Expr* e;
    Token t;

    // unop
    t = runtime.current_token;
    if (parser_accept(T_PLUS) || parser_accept(T_MINUS)) {
        e = consume_factor();
        return build_expr_unop(t, e);
    }

    // numlit
    if (t.type == T_NUMBER) {
        return consume_numlit();
    }

    // ( expr )
    if (parser_accept(T_OPEN)) {
        e = consume_expr();
        parser_expect(T_CLOSE);
        return e;
    }

    apc_return(E_PARSE_ERROR);
    // unreachable
    return NULL;
}

Expr* consume_term() {
    Expr* term;
    Expr* factor_n;
    Token op;

    term = consume_factor();

    while (runtime.current_token.type == T_STAR
    || runtime.current_token.type == T_SLASH) {
        op = runtime.current_token;
        if (!parser_next_token()) {
            apc_return(E_PARSE_ERROR);
        }

        // term *= factor_n
        factor_n = consume_factor();
        // left associative
        // 5 + 6 - 7 => -(+(5,6), 7)
        term = build_expr_binop(op, term, factor_n);
    }
    return term;
}

Expr* consume_expr() {
    Expr* expr;
    Expr* term_n;
    Token op;

    expr = consume_term();

    while(runtime.current_token.type == T_PLUS
    || runtime.current_token.type == T_MINUS
    || runtime.current_token.type == T_PERCENT
    || runtime.current_token.type == T_CONV) {
        op = runtime.current_token;
        if (!parser_next_token()) {
            apc_return(E_PARSE_ERROR);
        }

        // conv requires an explicit base literal afterward
        if (op.type == T_CONV) {
            term_n = consume_numlit();
        } else {
            term_n = consume_term();
        }

        expr = build_expr_binop(op, expr, term_n);
    }
    return expr;
}

Expr* build_expr_num(Token num, const Token* opt_base) {

    Bignum bn = {0};
    uint32_t base = BN_BASE_DEFAULT;

    if (opt_base != NULL) {
        // try to parse explicit base
        char* base_str = opt_base->atom.str;
        size_t base_len = opt_base->atom.len;
        if (!bnu_parse_digit(base_str, 0, base_len, 10, &base)) {
            apc_return(E_PARSE_ERROR);
        }
    }

    if (!bn_write3(&bn, num.atom.str, num.atom.len, base)) {
        apc_return(E_PARSE_ERROR);
    }

    Expr* e = expr_new();
    e->type = X_VALUE;
    e->value.type = V_NUMBER;
    e->value.number = bn;

    return e;
}

Expr* build_expr_unop(Token op, Expr* arg) {
    Expr* e = expr_new();
    e->type = X_UNOP;
    e->unop.data = rt_get_unop(op.atom.str[0]);
    e->unop.arg = arg;
    return e;
}

Expr* build_expr_binop(Token op, Expr* arg0, Expr* arg1) {
    Expr* e = expr_new();
    e->type = X_BINOP;
    e->binop.data = rt_get_binop(op.atom.str[0]);
    e->binop.arg0 = arg0;
    e->binop.arg1 = arg1;
    return e;
}

Value eval_expr(const Expr* e) {
    if (e->type == X_VALUE) {
        return e->value;
    } else if (e->type == X_UNOP) {
        Value v0 = eval_expr(e->unop.arg);
        Value v = e->unop.data->fn(v0);
        return v;
    } else if (e->type == X_BINOP) {
        Value v0 = eval_expr(e->binop.arg0);
        Value v1 = eval_expr(e->binop.arg1);
        Value v = e->binop.data->fn(v0, v1);
        return v;
    }

    apc_return(E_INTERNAL_ERROR);
    // unreachable
    return (Value){0};
}

