#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include <stdbool.h>
#include <stddef.h>

struct shell_parser {
    char *str;
    char *cur;
};

void
shell_parser_init(struct shell_parser *parser, char *str);

bool
shell_parser_pop(struct shell_parser *parser, size_t size, char *dest);

#endif

#ifdef SHELL_PARSER_IMPLEMENTATION

void
shell_parser_init(struct shell_parser *parser, char *str) {
    parser->str = str;
    parser->cur = str;
}

bool
shell_parser_pop(struct shell_parser *parser, size_t size, char *dest) {
    char *src = parser->cur;

    while(*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r') {
        src++;
    }

    if(*src == 0) {
        parser->cur = src;
        return false;
    }

    if(size < 1) {
        return false;
    }

    size_t written = 0;
    size_t max_write = size - 1;

    char in_quote = 0;

    while(*src) {
        if(in_quote) {
            if(*src == in_quote) {
                in_quote = 0;
                src++;
            } else if(*src == '\\' && in_quote == '"' && (*(src + 1) == '"' || *(src + 1) == '\\')) {
                src++;
                if(written < max_write) {
                    dest[written++] = *src;
                }
                src++;
            } else {
                if(written < max_write) {
                    dest[written++] = *src;
                }
                src++;
            }
        } else {
            if(*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r') {
                break;
            } else if(*src == '\'' || *src == '"') {
                in_quote = *src;
                src++;
            } else if(*src == '\\' && *(src + 1) != 0) {
                src++;
                if(written < max_write) {
                    dest[written++] = *src;
                }
                src++;
            } else {
                if(written < max_write) {
                    dest[written++] = *src;
                }
                src++;
            }
        }
    }

    dest[written] = 0;
    parser->cur = src;

    return true;
}

#endif
