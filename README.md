# Process-Activity-Firewall

### Steps to use the library:

1. Clone the repository and make sure you have CMake installed (version 3.20.3 or above).
2. Make a dir _build_ in the root directory and navigate into it using ``mkdir build && cd build``. Then finally issue ``cmake ../ && cmake --build .`` to compile the executables.
3. Load the library by issuing the command: ``export LD_PRELOAD=$PWD/src/libintercept.so``.
4. Run ``./src/listener`` in another shell, all the intercept logs will be displayed here.
5. To use the _substring_ feature of the listener, pass the substring to match as an argument to the listener executable as an command line argument. For example, ``./src/listener`` prints all the logs while ``./src/listener open`` prints logs that contain the substring _open_.

### Config details:

We have three config settings currently that can be set according the user's preference. An example ``config.json`` as well as ``logs.json`` is given in the root directory.

1. ``mode``: This sets whether none, all, all from list, all except from list files would be chosen. Defaults to 3.
    - 0 - _none_
    - 1 - _all_
    - 2 - _all from list_
    - 3 - _all except from list_
2. ``filepaths``: This is a list of filepaths as strings, these will be used according to the ``mode`` set. Contains _"/dev/pts/0"_ by default.
3. ``verbose``: This boolean setting decides whether logs are shown in stdout. Defaults to _true_. 
4. ``logfile_path``: This sets the path for the file where the logs will be writtten in JSON format.

### External libraries used:
- [lwjson](https://github.com/MaJerle/lwjson/tree/master) to parse the JSON files.
