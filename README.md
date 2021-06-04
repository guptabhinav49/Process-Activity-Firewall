# Process-Activity-Firewall

Steps to use the library:
1) Clone the repository, to compile the shared library use ``gcc -shared -fPIC inspect_open.c -o inspect_open.so -ldl``. Also, compile the listner using ``gcc listener.c -o listener``. Both of these tasks can be done in a single step by running ``./build.sh`` (first give permission using ``chmod 777 build.sh``).
2) Load the library by issuing the command: ``export LD_PRELOAD=$PWD/inspect_open.so`` 
3) To use the _substring_ feature of the listener, just pass the substring to match as an argument the listener object. For example, ``./listener`` prints all the logs while ``./listener open`` prints logs that contain the substring _open_.