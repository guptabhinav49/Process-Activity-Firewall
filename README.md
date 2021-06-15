# Process-Activity-Firewall

### Steps to use the library:

1. Clone the repository and make sure you have CMake installed (version 3.20.3 or above).
2. Make a dir _build_ in the root directory and navigate into it using ``mkdir build && cd build``. Then finally issue ``cmake ../ && cmake --build .`` to compile the executables.
3. Load the library by issuing the command: ``export LD_PRELOAD=$PWD/libintercept.so``.
4. Run ``./listener`` in another shell, all the intercept logs will be displayed here.
5. To use the _substring_ feature of the listener, pass the substring to match as an argument to the listener executable as an command line argument. For example, ``./listener`` prints all the logs while ``./listener open`` prints logs that contain the substring _open_.

### External libraries used:
- [lwjson](https://github.com/MaJerle/lwjson/tree/master)