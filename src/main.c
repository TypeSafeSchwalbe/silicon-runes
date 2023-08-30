
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "runtime.h"

int main(int argc, char** argv) {
    srand(time(NULL));

    Stack p = stack_new();
    Stack s = stack_new();
    interpret(&p, &s, "(1)((> )Ip1,?)@");
    stack_free(&p);
    stack_free(&s);

    return 0;
}