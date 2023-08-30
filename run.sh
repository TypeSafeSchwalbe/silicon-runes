
clear

mkdir debug > /dev/null 2>&1

gcc -Wextra src/*.c -o debug/silicon-runes -lm &&
out/silicon-runes