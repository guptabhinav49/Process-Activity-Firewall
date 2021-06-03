gcc -shared -fPIC intercept.c -o intercept.so -ldl;
gcc listener.c -o listener