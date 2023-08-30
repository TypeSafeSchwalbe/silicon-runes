
#pragma once

#include <stdlib.h>


typedef struct Stack {
    struct Value* values;
    size_t size;
    size_t malloc_size;
} Stack;

Stack stack_new();
void stack_push(Stack* s, struct Value v);
void stack_set(Stack* s, size_t i, struct Value v);
struct Value* stack_get(Stack* s, size_t i);
void stack_pop(Stack* s);
void stack_free(Stack* s);


typedef struct Value {
    enum {
        Int,
        Float,
        String,
        Array
    } type;
    union {
        long int i;
        double f;
        char* s;
        Stack* a;
    } value; 
} Value;

Value value_int(long int v);
Value value_float(double v);
Value value_string(char* v);
Value value_array(Stack* v);
Value value_copy(Value* v);
void value_print(Value* v);
void value_free(Value* v);


void interpret(Stack* primary, Stack* secondary, char* expression);