
clear

mkdir out > /dev/null 2>&1

gcc -Wextra src/*.c -o out/silicon-runes -lm -O3 &&
out/silicon-runes