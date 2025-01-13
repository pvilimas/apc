#include "apc.h"

bool char_in_string(char c, const char* str) {
    for (const char* s = str; *s != '\0'; s++) {
        if (*s == c) {
            return true;
        }
    }
    return false;
}

bool parens_are_balanced(stringview sv) {
    int64_t count = 0;
    for (size_t i = 0; i < sv.len; i++) {
        char c = sv.str[i];
        if (c == '(') count += 1;
        else if (sv.str[i] == ')') {
            count -= 1;
            if (count < 0) {
                return false;
            }
        }
    }

    return count == 0;
}

void signal_handler(int s) {
    (void)s;
    fputc('\n', stdout);
    apc_exit(0);
}
