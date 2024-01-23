static bool         scanner__is_at_end(scanner_t* self);
static token_t      scanner__make_token(scanner_t* self, token_type_t type);
static char         scanner__peak(scanner_t* self);
static char         scanner__eat(scanner_t* self);
static bool         scanner__eat_if(scanner_t* self, char expected);
static void         scanner__skip_whitespaces(scanner_t* self);
static token_t      scanner__make_string(scanner_t* self);
static token_t      scanner__make_number(scanner_t* self, bool seen_dor);
static token_t      scanner__make_identifier(scanner_t* self);
static token_type_t scanner__identifier_type(scanner_t* self);

static bool scanner__is_digit(char c);
static bool scanner__is_alpha(char c);

static bool scanner__is_at_end(scanner_t* self) {
    return scanner__peak(self) == '\0' || self->cur == self->source_end;
}

static token_t scanner__make_token(scanner_t* self, token_type_t type) {
    token_t result = {
        .lexeme     = self->start,
        .lexeme_len = (uint32_t) (self->cur - self->start),
        .type       = type,
        .line_s     = self->line_s,
        .line_e     = self->line_e,
        .col_s      = self->col_s,
        .col_e      = self->col_e
    };

#if 0
    fprintf(
        stderr,
        "%s: [%.*s] %u:%u - %u:%u\n",
        token_type__to_str(result.type), result.lexeme_len, result.lexeme, result.line_s, result.col_s, result.line_e, result.col_e
    );
#endif

    return result;
}

static char scanner__peak(scanner_t* self) {
    return *self->cur;
}

static char scanner__eat(scanner_t* self) {
    ++self->col_e;
    return *self->cur++;
}

static bool scanner__eat_if(scanner_t* self, char expected) {
    if (scanner__is_at_end(self) || scanner__peak(self) != expected) {
        return false;
    }

    ++self->cur;

    return true;
}

static token_t scanner__make_comment(scanner_t* self) {
    switch (scanner__peak(self)) {
        case '/': {
            do {
                scanner__eat(self);
            } while (scanner__peak(self) != '\n' && !scanner__is_at_end(self));
            return scanner__make_token(self, TOKEN_COMMENT);
        } break ;
        case '*': {
            scanner__eat(self);
            do {
                char c = scanner__eat(self);

                if (c == '*' && scanner__eat_if(self, '/')) {
                    return scanner__make_token(self, TOKEN_COMMENT);
                } else if (c == '\n') {
                    ++self->line_e;
                    self->col_e = 1;
                }
            } while (!scanner__is_at_end(self));
        } break ;
        default: assert(false);
    }

    return scanner__make_token(self, TOKEN_ERROR);
}

static void scanner__skip_whitespaces(scanner_t* self) {
    while (true) {
        switch (scanner__peak(self)) {
            case ' ':
            case '\t':
            case '\r': {
                scanner__eat(self);
            } break ;
            case '\n': {
                ++self->line_e;
                self->col_e = 1;
                scanner__eat(self);
            } break ;
            default: return;
        }
    }
}

static token_t scanner__make_string(scanner_t* self) {
    while (scanner__peak(self) != '"' && !scanner__is_at_end(self)) {
        if (scanner__peak(self) == '\n') {
            ++self->line_e;
            self->col_e = 1;
        }
        scanner__eat(self);
    }

    if (scanner__is_at_end(self)) {
        return scanner__make_token(self, TOKEN_ERROR);
    }

    // Closing quote
    scanner__eat(self);

    return scanner__make_token(self, TOKEN_STRING);
}

static token_t scanner__make_number(scanner_t* self, bool seen_dot) {
    while (scanner__is_digit(scanner__peak(self))) {
        scanner__eat(self);
    }

    if (!seen_dot && scanner__eat_if(self, '.')) {
        seen_dot = true;
    }

    while (scanner__is_digit(scanner__peak(self))) {
        scanner__eat(self);
    }

    if (seen_dot) {
        return scanner__make_token(self, TOKEN_R32);
    } else {
        return scanner__make_token(self, TOKEN_S32);
    }
}

static token_t scanner__make_identifier(scanner_t* self) {
    while (scanner__is_alpha(scanner__peak(self)) || scanner__is_digit(scanner__peak(self))) {
        scanner__eat(self);
    }

    return scanner__make_token(self, scanner__identifier_type(self));
}

static token_type_t scanner__identifier_type_helper(scanner_t* self, uint32_t start, const char* rest, token_type_t type) {
    uint32_t len = strlen(rest);
    if ((uint32_t) (self->cur - self->start != start + len)) {
        return TOKEN_ERROR;
    }

    if (strncmp(self->start + start, rest, len) == 0) {
        return type;
    }

    return TOKEN_ERROR;
}

static token_type_t scanner__identifier_type(scanner_t* self) {
    switch (*self->start) {
        case 'v': return scanner__identifier_type_helper(self, 1, "ersion", TOKEN_VERSION);
        case 'p': return scanner__identifier_type_helper(self, 1, "ositions", TOKEN_POSITION);
        case 'n': return scanner__identifier_type_helper(self, 1, "ormals", TOKEN_NORMAL);
        case 't': return scanner__identifier_type_helper(self, 1, "extures_2d", TOKEN_TEXTURE_2D);
        case 'm': return scanner__identifier_type_helper(self, 1, "aterials", TOKEN_MATERIAL);
        case 'i': return scanner__identifier_type_helper(self, 1, "ndices", TOKEN_INDEX);
        case 'g': return scanner__identifier_type_helper(self, 1, "eometry", TOKEN_GEOMETRY);
    }

    return TOKEN_ERROR;
}

static bool scanner__is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool scanner__is_alpha(char c) {
    return (c >= 'a'&& c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
