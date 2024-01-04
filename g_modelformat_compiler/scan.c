#include "scan.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "scan_impl.c"

void scanner__create(scanner_t* self, const char* source, size_t source_len) {
    self->source_start = source;
    self->source_end   = source + source_len;
    self->start        = source;
    self->cur          = source;
    self->line_s       = 1;
    self->line_e       = 1;
    self->col_s        = 1;
    self->col_e        = 1;
}

token_t scanner__next_token(scanner_t* self) {
    scanner__skip_whitespaces(self);

    self->start  = self->cur;
    self->line_s = self->line_e;
    self->col_s  = self->col_e;

    if (scanner__is_at_end(self)) {
        return scanner__make_token(self, TOKEN_EOF);
    }

    char c = scanner__eat(self);

    if (scanner__is_alpha(c)) {
        return scanner__make_identifier(self);
    }

    if (scanner__is_digit(c)) {
        return scanner__make_number(self, false);
    }

    switch (c) {
        case '(': return scanner__make_token(self, TOKEN_LEFT_PAREN);
        case ')': return scanner__make_token(self, TOKEN_RIGHT_PAREN);
        case '.': {
            if (scanner__is_digit(scanner__peak(self))) {
                return scanner__make_number(self, true);
            }
        } break ;
        case '-': {
            char next_c = scanner__peak(self);
            if (next_c == '.' || scanner__is_digit(next_c)) {
                return scanner__make_number(self, false);
            }
        } break ;
        case '/': {
            switch (scanner__peak(self)) {
                case '*':
                case '/': return scanner__make_comment(self);
            }
        } break ;
        case '"': return scanner__make_string(self);
    }

    return scanner__make_token(self, TOKEN_ERROR);
}

const char* token_type__to_str(token_type_t token_type) {
    switch (token_type) {
    case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
    case TOKEN_STRING: return "STRING";
    case TOKEN_NUMBER: return "NUMBER";
    case TOKEN_VERSION: return "VERSION";
    case TOKEN_POSITION: return "POSITION";
    case TOKEN_NORMAL: return "NORMAL";
    case TOKEN_TEXTURE_2D: return "TEXTURE_2D";
    case TOKEN_MATERIAL: return "MATERIAL";
    case TOKEN_INDEX: return "INDEX";
    case TOKEN_COMMENT: return "COMMENT";
    case TOKEN_ERROR: return "ERROR";
    case TOKEN_EOF: return "EOF";
    default: assert(false);
    }
    return 0;
}
