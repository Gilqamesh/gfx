#ifndef SCAN_H
# define SCAN_H

# include <stdint.h>
# include <stddef.h>

struct         scanner;
enum           token_type;
struct         token;
typedef struct scanner    scanner_t;
typedef enum   token_type token_type_t;
typedef struct token      token_t;

struct scanner {
    const char*  source_start;
    const char*  source_end;
    const char*  start;
    const char*  cur;
    uint32_t     line_s;
    uint32_t     line_e;
    uint32_t     col_s;
    uint32_t     col_e;
};

enum token_type {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,

    TOKEN_STRING,
    TOKEN_S32, /* 2 -2 */
    TOKEN_R32, /* -2.  2.2  .3 */

    TOKEN_VERSION,
    TOKEN_POSITION,
    TOKEN_NORMAL,
    TOKEN_TEXTURE_2D,
    TOKEN_MATERIAL,
    TOKEN_INDEX,
    TOKEN_GEOMETRY,

    TOKEN_COMMENT,

    TOKEN_ERROR,
    TOKEN_EOF
};

struct token {
    const char*  lexeme; // not null-terminated
    uint32_t     lexeme_len;
    token_type_t type;
    uint32_t     line_s;
    uint32_t     line_e;
    uint32_t     col_s;
    uint32_t     col_e;
};

void scanner__create(scanner_t* self, const char* source, size_t source_len);
token_t scanner__next_token(scanner_t* self);

const char* token_type__to_str(token_type_t token_type);

float   token__to_r32(token_t* self);
int32_t token__to_s32(token_t* self);

#endif // SCAN_H
