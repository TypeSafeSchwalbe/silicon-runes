
clear

mkdir release > /dev/null 2>&1

mkdir release/linux > /dev/null 2>&1
gcc src/*.c -o release/linux/silicon-runes -lm -O3

mkdir release/windows > /dev/null 2>&1
x86_64-w64-mingw32-gcc src/*.c -o release/windows/silicon-runes.exe -lm -O3

mkdir release/macosx > /dev/null 2>&1
export PATH="/home/devtaube/osxcross/target/bin:$PATH"
x86_64-apple-darwin14-clang -Wno-everything src/*.c -o release/macosx/silicon-runes -lm -O3 
