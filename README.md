# Process-Activity-Firewall

Steps to use the library:
1) Clone the repository, to compile the shared library use ``gcc -shared -fPIC inspect_open.c -o inspect_open.so -ldl``.
2) Load the library by issuing the command: ``export LD_PRELOAD=$PWD/inspect_open.so`` 