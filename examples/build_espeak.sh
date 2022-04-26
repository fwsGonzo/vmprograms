clang++-12 -static -O2 -std=c++17 espeak.cpp -L/usr/local/lib -lespeak-ng -pthread -o espeak
strip --strip-all espeak
ls -la espeak
../upload.sh espeak ypizza.com
