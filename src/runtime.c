
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "runtime.h"
#include "error.h"


Value value_int(long int v) { return (Value) { .type = Int, .value = { .i = v } }; }
Value value_float(double v) { return (Value) { .type = Float, .value = { .f = v } }; }
Value value_string(char* v) {
    Value n = (Value) { .type = String, .value = { .s = malloc(strlen(v) + 1) } };
    strcpy(n.value.s, v);
    return n;
}
Value value_array(Stack* v) { return (Value) { .type = Array, .value = { .a = v } }; }
Value value_copy(Value* v) {
    Value c;
    memcpy(&c, v, sizeof(Value));
    switch(c.type) {
        case String: {
            c.value.s = malloc(strlen(v->value.s) + 1);
            strcpy(c.value.s, v->value.s);
        } break;
        case Array: {
            c.value.a = malloc(sizeof(Stack));
            memcpy(c.value.a, v->value.a, sizeof(Stack));
            c.value.a->values = malloc(c.value.a->malloc_size * sizeof(Value));
            memcpy(c.value.a->values, v->value.a->values, c.value.a->size * sizeof(Value));
        } break;
    }
    return c;
}
void value_print(Value* v) {
    switch(v->type) {
        case Int: printf("%ld", v->value.i); break;
        case Float: printf("%f", v->value.f); break;
        case String: printf("%s", v->value.s); break;
        case Array: {
            printf("[");
            for(size_t i = 0; i < v->value.a->size; i += 1) {
                if(i > 0) { printf(", "); }
                value_print(stack_get(v->value.a, i));
            }
            printf("]");
        } break;
    }
}
void value_free(Value* v) {
    switch(v->type) {
        case String: free(v->value.s); break;
        case Array: {
            stack_free(v->value.a);
            free(v->value.a);
        } break;
    }
}


Stack stack_new() {
    Stack s;
    s.malloc_size = 16;
    s.size = 0;
    s.values = malloc(s.malloc_size * sizeof(Value));
    return s;
}
void stack_push(Stack* s, Value v) {
    s->size += 1;
    if(s->size > s->malloc_size) {
        s->malloc_size *= 2;
        s->values = realloc(s->values, s->malloc_size * sizeof(Value));
    }
    stack_set(s, s->size - 1, v);
}
inline void stack_set(Stack* s, size_t i, Value v) {
    memcpy(&s->values[i], &v, sizeof(Value));
}
inline Value* stack_get(Stack* s, size_t i) { return &s->values[i]; }
inline void stack_pop(Stack* s) { s->size -= 1; }
void stack_free(Stack* s) {
    for(size_t v = 0; v < s->size; v += 1) {
        value_free(stack_get(s, v));
    }
    free(s->values);
}


#define INVALID_INSTRUCTION_FMT(c) "'%c' is not a valid instruction!", c

#define GET_INFIX_ARGS()\
    if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }\
    Value b = *stack_get(primary, primary->size - 1);\
    Value a = *stack_get(primary, primary->size - 2);\
    if(b.type != Int && b.type != Float) { report_error("the first item is not a number", primary, secondary, expression, i_ptr); }\
    if(a.type != Int && a.type != Float) { report_error("the second item is not a number", primary, secondary, expression, i_ptr); }

#define NUMBER_INFIX_OP(OP)\
    stack_pop(primary);\
    stack_pop(primary);\
    if(a.type == Float || b.type == Float) {\
        stack_push(primary, value_float((a.type == Float? a.value.f : a.value.i) OP (b.type == Float? b.value.f : b.value.i)));\
    } else {\
        stack_push(primary, value_int((a.value.i) OP (b.value.i)));\
    }\
    value_free(&a);\
    value_free(&b);

void interpret(Stack* primary, Stack* secondary, char* expression) {
    char* i_ptr = expression;
    while(*i_ptr != '\0') {
        switch(*i_ptr) {
            // no op
            case ' ': case '\n': break;

            // push number onto primary stack
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
                char* start = i_ptr;
                while('0' <= *i_ptr && *i_ptr <= '9') {
                    i_ptr += 1;
                }
                int is_float = 0;
                if(i_ptr != start && *i_ptr == '.' && '0' <= *(i_ptr + 1) && *(i_ptr + 1) <= '9') {
                    i_ptr += 1;
                    is_float = 1;
                }
                while('0' <= *i_ptr && *i_ptr <= '9') {
                    i_ptr += 1;
                }
                size_t number_length = i_ptr - start;
                char number[number_length + 1];
                memcpy(number, start, number_length);
                number[number_length] = '\0';
                if(is_float) {
                    stack_push(primary, value_float(strtod(number, NULL)));
                } else {
                    stack_push(primary, value_int(strtol(number, NULL, 10)));
                }
                i_ptr -= 1; // will be increased again after the switch
            } break;
            // push paren content onto primary stack
            case '(': {
                i_ptr += 1;
                char* start = i_ptr;
                int scope = 1;
                while(scope != 1 || *i_ptr != ')') {
                    if(*i_ptr == '(') { scope += 1; }
                    if(*i_ptr == ')') { scope -= 1; }
                    i_ptr += 1;
                    if(*i_ptr == '\0') { report_error("unclosed string literal", primary, secondary, expression, i_ptr); }
                }
                size_t string_length = i_ptr - start;
                char* string = malloc(string_length + 1);
                memcpy(string, start, string_length);
                string[string_length] = '\0';
                stack_push(primary, value_string(string));
                // will be increased again after the switch
            } break;

            // push a copy of the top primary stack item onto the primary stack
            case ':': {
                if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                stack_push(primary, value_copy(stack_get(primary, primary->size - 1)));
            } break;
            // pop the top item off the primary stack
            case '^': {
                if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                value_free(stack_get(primary, primary->size - 1));
                stack_pop(primary);
            } break;
            // swap the top two items on the primary stack
            case '$': {
                if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value a = *stack_get(primary, primary->size - 1);
                Value b = *stack_get(primary, primary->size - 2);
                stack_set(primary, primary->size - 1, b);
                stack_set(primary, primary->size - 2, a);
            } break;
            // pop the top item off the primary stack and push it onto the secondary stack
            case '#': {
                if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value moved = *stack_get(primary, primary->size - 1);
                stack_pop(primary);
                stack_push(secondary, moved);
            } break;
            // pop the top item off the secondary stack and push it onto the primary stack
            case '\'': {
                if(secondary->size < 1) { report_error("the secondary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value moved = *stack_get(secondary, secondary->size - 1);
                stack_pop(secondary);
                stack_push(primary, moved);
            } break;

            // receive text as input and push it onto the primary stack
            case ',': {
                int content_ms = 64;
                char* content = malloc(content_ms + 1);
                int ci = 0;
                for(;;) {
                    int c = fgetc(stdin);
                    if(c == EOF || c == '\n') { break; }
                    if(ci >= content_ms) {
                        content_ms *= 2;
                        content = realloc(content, content_ms + 1);
                    }
                    content[ci] = c;
                    ci += 1;
                }
                content = realloc(content, ci + 1);
                content[ci] = '\0';
                stack_push(primary, value_string(content));
            } break;
            // pop the top item off the primary stack and print it
            case '!': {
                if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value v = *stack_get(primary, primary->size - 1);
                stack_pop(primary);
                value_print(&v);
                printf("\n");
                value_free(&v);
            } break;

            // pop the top two stack items off the primary stack and push their sum onto the primary stack
            case '+': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(+)
            } break;
            // pop the top two stack items off the primary stack and push their difference onto the primary stack
            case '-': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(-)
            } break;
            // pop the top two stack items off the primary stack and push their product onto the primary stack
            case '*': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(*)
            } break;
            // pop the top two stack items off the primary stack and push their quotient onto the primary stack
            case '/': {
                GET_INFIX_ARGS()
                if(b.type == Int && b.value.i == 0) { report_error("integer division by zero", primary, secondary, expression, i_ptr); }
                NUMBER_INFIX_OP(/)
            } break;
            // pop the top two stack items off the primary stack and push their remainder onto the primary stack
            case '%': {
                if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value b = *stack_get(primary, primary->size - 1);
                stack_pop(primary);
                Value a = *stack_get(primary, primary->size - 1);
                stack_pop(primary);
                if(b.type != Int && b.type != Float) { report_error("the first item is not a number", primary, secondary, expression, i_ptr); }
                if(a.type != Int && a.type != Float) { report_error("the second item is not a number", primary, secondary, expression, i_ptr); }
                if(a.type == Float || b.type == Float) {
                    stack_push(primary, value_float(fmod(a.type == Float? a.value.f : a.value.i, b.type == Float? b.value.f : b.value.i)));
                } else {
                    stack_push(primary, value_int(a.value.i % b.value.i));
                }
                value_free(&a);
                value_free(&b);
            } break;

            // pop the top two stack items off the primary stack. if the first is less than the second, push 1 (otherwise 0) onto the primary stack.
            case '<': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(<)
            } break;
            // pop the top two stack items off the primary stack. if the first is greater than the second, push 1 (otherwise 0) onto the primary stack.
            case '>': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(>)
            } break;
            // pop the top two stack items off the primary stack. if they are equal, push 1 (otherwise 0) onto the primary stack.
            case '=': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(==)
            } break;

            // pop the top two stack items off the primary stack. if both are truthy, push 1 (otherwise 0) onto the primary stack.
            case '&': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(&&)
            } break;
            // pop the top two stack items off the primary stack. if at least one of them is truthy, push 1 (otherwise 0) onto the primary stack.
            case '|': {
                GET_INFIX_ARGS()
                NUMBER_INFIX_OP(||)
            } break;

            // pop the top item off the primary stack. if the (now) top stack item is truthy, evaluate the popped expression.
            case '?': {
                if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value e = *stack_get(primary, primary->size - 1);
                Value cv = *stack_get(primary, primary->size - 2);
                if(e.type != String) { report_error("the first item is not a string", primary, secondary, expression, i_ptr); }
                stack_pop(primary);
                stack_pop(primary);
                int truthy = 0;
                switch(cv.type) {
                    case Int: truthy = cv.value.i != 0; break;
                    case Float: truthy = cv.value.i != 0.0; break;
                    case String: truthy = strlen(cv.value.s) != 0; break;
                    case Array: truthy = cv.value.a->size != 0; break;
                }
                value_free(&cv);
                if(truthy) {
                    interpret(primary, secondary, e.value.s);
                }
                value_free(&e);
            } break;
            // pop the top item off the primary stack. repeatedly evaluate the popped expression while the (now) top stack item is truthy.
            case '@': {
                if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                Value e = *stack_get(primary, primary->size - 1);
                Value c = *stack_get(primary, primary->size - 2);
                if(e.type != String) { report_error("the first item is not a string", primary, secondary, expression, i_ptr); }
                if(c.type != String) { report_error("the second item is not a string", primary, secondary, expression, i_ptr); }
                stack_pop(primary);
                stack_pop(primary);
                for(;;) {
                    interpret(primary, secondary, c.value.s);
                    if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                    Value cv = *stack_get(primary, primary->size - 1);
                    stack_pop(primary);
                    int truthy = 0;
                    switch(cv.type) {
                        case Int: truthy = cv.value.i != 0; break;
                        case Float: truthy = cv.value.i != 0.0; break;
                        case String: truthy = strlen(cv.value.s) != 0; break;
                        case Array: truthy = cv.value.a->size != 0; break;
                    }
                    if(!truthy) {
                        break;
                    }
                    value_free(&cv);
                    interpret(primary, secondary, e.value.s);
                }
                value_free(&e);
                value_free(&c);
            } break;

            // array-related instruction
            case 'A': {
                i_ptr += 1;
                int b = 1;
                switch(*i_ptr) {
                    // *c*reate array
                    case 'c': {
                        Stack* c = malloc(sizeof(Stack));
                        *c = stack_new();
                        stack_push(primary, value_array(c));
                    } break;
                    // *p*ush onto array
                    case 'p': {
                        if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value v = *stack_get(primary, primary->size - 1);
                        Value* a = stack_get(primary, primary->size - 2);
                        if(a->type != Array) { report_error("the second item is not an array", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        stack_push(a->value.a, v);
                    } break;
                    // *g*et array index
                    case 'g': {
                        if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value i = *stack_get(primary, primary->size - 1);
                        Value* a = stack_get(primary, primary->size - 2);
                        if(i.type != Int) { report_error("the first item is not in integer", primary, secondary, expression, i_ptr); }
                        if(a->type != Array) { report_error("the second item is not an array", primary, secondary, expression, i_ptr); }
                        if(i.value.i < 0 || (size_t) i.value.i >= a->value.a->size) { report_error("the index is out of bounds", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        stack_push(primary, value_copy(stack_get(a->value.a, i.value.i)));
                    } break;
                    // *s*et array index
                    case 's': {
                        if(primary->size < 3) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value v = *stack_get(primary, primary->size - 1);
                        Value i = *stack_get(primary, primary->size - 2);
                        Value* a = stack_get(primary, primary->size - 3);
                        if(i.type != Int) { report_error("the first item is not in integer", primary, secondary, expression, i_ptr); }
                        if(a->type != Array) { report_error("the second item is not an array", primary, secondary, expression, i_ptr); }
                        if(i.value.i < 0 || (size_t) i.value.i >= a->value.a->size) { report_error("the index is out of bounds", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        stack_pop(primary);
                        stack_set(a->value.a, i.value.i, v);
                    } break;
                    // *r*emove array index
                    case 'r': {
                        if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value i = *stack_get(primary, primary->size - 1);
                        Value* a = stack_get(primary, primary->size - 2);
                        if(i.type != Int) { report_error("the first item is not in integer", primary, secondary, expression, i_ptr); }
                        if(a->type != Array) { report_error("the second item is not an array", primary, secondary, expression, i_ptr); }
                        if(i.value.i < 0 || (size_t) i.value.i >= a->value.a->size) { report_error("the index is out of bounds", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        for(size_t v = i.value.i + 1; v < a->value.a->size; v += 1) {
                            stack_set(a->value.a, v - 1, *stack_get(a->value.a, v));
                        }
                        stack_pop(a->value.a);
                    } break;
                    // *l*ength of array
                    case 'l': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* a = stack_get(primary, primary->size - 1);
                        if(a->type != Array) { report_error("the first item is not an array", primary, secondary, expression, i_ptr); }
                        stack_push(primary, value_int(a->value.a->size));
                    } break;

                    default: b = 0;
                }
                if(b) { break; }
            }
            // fall through

            // interpreter-related instruction
            case 'I': {
                i_ptr += 1;
                int b = 1;
                switch(*i_ptr) {
                    // *r*eset the stacks
                    case 'r': {
                        stack_free(primary);
                        stack_free(secondary);
                        *primary = stack_new();
                        *secondary = stack_new();
                    } break;
                    // *p*rint raw string
                    case 'p': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value s = *stack_get(primary, primary->size - 1);
                        if(s.type != String) { report_error("the first item is not a string", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        printf("%s", s.value.s);
                        value_free(&s);
                    } break;
                    // print *d*ebug information
                    case 'd': {
                        printf("[Stack]");
                        printf("\nprimary:");
                        if(primary->size > 0) {
                            for(size_t v = 0; v < primary->size; v += 1) {
                                printf(" [%ld] ", v);
                                value_print(stack_get(primary, v));
                            }
                        } else {
                            printf(" <empty>");
                        }
                        printf("\nsecondary:");
                        if(secondary->size > 0) {
                            for(size_t v = 0; v < secondary->size; v += 1) {
                                printf(" [%ld] ", v);
                                value_print(stack_get(secondary, v));
                            }
                        } else {
                            printf(" <empty>");
                        }
                        printf("\n");
                    } break;
                    // push *p*rimary stack size (before call) onto the primary stack
                    case 'P': {
                        stack_push(primary, value_int(primary->size));
                    } break;
                    // push *s*econdary stack size onto the primary stack
                    case 'S': {
                        stack_push(primary, value_int(secondary->size));
                    } break;

                    default: b = 0;
                }
                if(b) { break; }
            }
            // fall through

            // string-related instruction
            case 'S': {
                i_ptr += 1;
                int b = 1;
                switch(*i_ptr) {
                    // *m*erge strings
                    case 'm': {
                        if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value b = *stack_get(primary, primary->size - 1);
                        Value a = *stack_get(primary, primary->size - 2);
                        if(b.type != String) { report_error("the first item is not a string", primary, secondary, expression, i_ptr); }
                        if(a.type != String) { report_error("the second item is not a string", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        stack_pop(primary);
                        size_t a_length = strlen(a.value.s);
                        size_t b_length = strlen(b.value.s);
                        char* merged = malloc(a_length + b_length + 1);
                        memcpy(merged,            a.value.s, a_length);
                        memcpy(merged + a_length, b.value.s, b_length);
                        merged[a_length + b_length] = '\0';
                        value_free(&b);
                        value_free(&a);
                        stack_push(primary, value_string(merged));
                    } break;
                    // create *s*ubstring copy
                    case 's': {
                        if(primary->size < 3) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value end = *stack_get(primary, primary->size - 1);
                        Value start = *stack_get(primary, primary->size - 2);
                        Value* s = stack_get(primary, primary->size - 3);
                        if(start.type != Int) { report_error("the first item is not an integer", primary, secondary, expression, i_ptr); }
                        if(end.type != Int) { report_error("the second item is not an integer", primary, secondary, expression, i_ptr); }
                        if(s->type != String) { report_error("the third item is not a string", primary, secondary, expression, i_ptr); }
                        size_t s_length = strlen(s->value.s);
                        if(start.value.i < 0 || (size_t) start.value.i >= s_length) { report_error("the start index is out of bounds", primary, secondary, expression, i_ptr); }
                        if(end.value.i < 0 || (size_t) end.value.i >= s_length) { report_error("the end index is out of bounds", primary, secondary, expression, i_ptr); }
                        if(end.value.i < start.value.i) { report_error("the end index is smaller than the start index", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        stack_pop(primary);
                        size_t sub_length = end.value.i - start.value.i;
                        char* sub = malloc(sub_length + 1);
                        memcpy(sub, s->value.s + start.value.i, sub_length);
                        sub[sub_length] = '\0';
                        stack_push(primary, value_string(sub));
                    } break;
                    // get string *l*ength
                    case 'l': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* s = stack_get(primary, primary->size - 1);
                        if(s->type != String) { report_error("the first item is not a string", primary, secondary, expression, i_ptr); }
                        stack_push(primary, value_int(strlen(s->value.s)));
                    } break;

                    default: b = 0;
                }
                if(b) { break; }
            }
            // fall through

            // math-related instruction
            case 'M': {
                i_ptr += 1;
                int b = 1;
                switch(*i_ptr) {
                    // put *P*i onto the stack
                    case 'P': {
                        stack_push(primary, value_float(3.14159265358979323846));
                    } break;
                    // put *T*au onto the stack
                    case 'T': {
                        stack_push(primary, value_float(6.28318530717958647692));
                    } break;
                    // put *E*uler's number onto the stack
                    case 'E': {
                        stack_push(primary, value_float(2.7182818284590452354));
                    } break;
                    // put a *r*andom number that is greater or equal to 0 and less than 1 onto the stack
                    case 'R': {
                        stack_push(primary, value_float((float) rand() / (float) RAND_MAX));
                    } break;
                         
                    // convert integer to *f*loat
                    case 'f': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Int) { report_error("the first item is not an integer", primary, secondary, expression, i_ptr); }
                        x->value.f = (float) x->value.i;
                        x->type = Float;
                    } break;
                    // round number at the top of the stack *u*p
                    case 'u': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.i = (int) ceil(x->value.f);
                        x->type = Int;
                    } break;
                    // round number at the top of the stack *d*own
                    case 'd': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.i = (int) floor(x->value.f);
                        x->type = Int;
                    } break;
                    // round number at the top of the stack to the *n*earest integer
                    case 'n': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.i = (int) round(x->value.f);
                        x->type = Int;
                    } break;
                    // calulate the *s*ine of the number at the top of the stack
                    case 's': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.f = sin(x->value.f);
                    } break;
                    // calculate the *c*osine of the number at the top of the stack
                    case 'c': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.f = cos(x->value.f);
                    } break;
                    // calculate the *t*angent of the number at the top of the stack
                    case 't': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.f = tan(x->value.f);
                    } break;
                    // calculate the *a*bsolute value of the number at the top of the stack
                    case 'a': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        switch(x->type) {
                            case Int: x->value.i = labs(x->value.i); break;
                            case Float: x->value.f = fabs(x->value.f); break;
                            default: report_error("the first item is not an integer or float", primary, secondary, expression, i_ptr);
                        }
                    } break;
                    // calculate the square *r*oot of the number at the top of the stack
                    case 'r': {
                        if(primary->size < 1) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value* x = stack_get(primary, primary->size - 1);
                        if(x->type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        x->value.f = sqrt(x->value.f);
                    } break;
                    // calculate the second number on the stack (top - 1) to the power of the first (top), replace with result
                    case 'p': {
                        if(primary->size < 2) { report_error("the primary stack does not contain enough items", primary, secondary, expression, i_ptr); }
                        Value n = *stack_get(primary, primary->size - 1);
                        Value* x = stack_get(primary, primary->size - 2);
                        if(n.type != Float) { report_error("the first item is not a float", primary, secondary, expression, i_ptr); }
                        if(x->type != Float) { report_error("the second item is not a float", primary, secondary, expression, i_ptr); }
                        stack_pop(primary);
                        x->value.f = pow(x->value.f, n.value.f);
                    } break;

                    default: b = 0;
                }
                if(b) { break; }
            }
            // fall through

            // invalid instruction
            default: {
                char error_reason[snprintf(NULL, 0, INVALID_INSTRUCTION_FMT(*i_ptr))]; 
                sprintf(error_reason, INVALID_INSTRUCTION_FMT(*i_ptr));
                report_error(error_reason, primary, secondary, expression, i_ptr);
            }
        }
        i_ptr += 1;
    }
}