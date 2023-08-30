
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"


void report_error(char* reason, Stack* primary, Stack* secondary, char* expression, char* i_ptr) {
    printf("[Error] %s\n", reason);
    printf("[Instruction]\n");
    {
        size_t pre_offset = i_ptr - expression;
        if(pre_offset > 3) { pre_offset = 3; }
        size_t post_offset = strlen(i_ptr);
        if(post_offset > 3) { post_offset = 3; }
        char displayed[pre_offset + 1 + post_offset + 1];
        memcpy(displayed, i_ptr - pre_offset, pre_offset + 1 + post_offset);
        displayed[pre_offset + 1 + post_offset] = '\0';
        printf("  %s\n", displayed);
        for(size_t i = 0; i < 2 + pre_offset; i += 1) { printf(" "); }
        printf("^\n");
    }
    printf("[Stack]\n");
    printf("  primary:\n");
    if(primary->size > 0) {
        for(size_t v = primary->size - 1;;) {
            printf("  | %ld: ", v);
            value_print(stack_get(primary, v));
            printf("\n");
            if(v <= 0) { break; }
            v -= 1;
        }
    } else {
        printf("    <empty>\n");
    }
    printf("  secondary:\n");
    if(secondary->size > 0) {
        for(size_t v = secondary->size - 1;;) {
            printf("  | %ld: ", v);
            value_print(stack_get(secondary, v));
            printf("\n");
            if(v == 0) { break; }
            v -= 1;
        }
    } else {
        printf("    <empty>\n");
    }
    exit(1);
}