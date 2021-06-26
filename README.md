# Process-Activity-Firewall

### Steps to use the library:

1. Clone the repository and make sure you have CMake installed (version 3.20.3 or above).
2. Make a dir _build_ in the root directory and navigate into it using ``mkdir build && cd build``. Then finally issue ``cmake ../ && cmake --build .`` to compile the executables.
3. Load the library by issuing the command: ``export LD_PRELOAD=$PWD/src/libintercept.so``.
4. Run ``./src/listener`` in another shell, all the intercept logs will be displayed here.
5. To use the _substring_ feature of the listener, pass the substring to match as an argument to the listener executable as an command line argument. For example, ``./src/listener`` prints all the logs while ``./src/listener open`` prints logs that contain the substring _open_.

### Config details:

We have four config settings currently that can be set according the user's preference. An example ``config.json`` as well as ``logs.json`` is given in the root directory.

1. ``filegroups``: This is a list of file groups. Each file group has a ``expr``, ``mode`` and ``type`` attributes. The firewall will monitor (or not) the files names given in ``expr`` depending on the ``mode`` attribute. The ``type`` attribute decides how the expressions are matched (for example, complete match or matching the regular expression with current file's absolute path). 
    - ``type`` - This sets how the string in the file group is compared to the absolute path the current file being monitored.
        - 0 - _exact match_
        - 1 - _filepaths matching the regular expressions_
        - 2 - _filenames matching the regular expressions_
    - ``expr`` - This is a list of strings (or regular expressions depending on the ``type`` attribute)
    - ``mode`` - This sets whether the current group should be ignored or, whether a matched file is shown or not shown.
        - 0 - _none_
        - 1 - _all from list_
        - 2 - _all except from list_

    Note that the regular expressions should be in POSIX (extended) format to work correctly. Also, if a file(name/path) matches 2 groups with different ``(type, mode)``, then the precedence is given to the exact match i.e. if there is some group with type _0_ and some expression in its ``exprs`` mathces then that group's ``mode`` is considered, while if there is no exact match then the first regular expression that matches (if any) decides the mode

2. ``ignore_filegroups`` : If set ``true``, all filegroups are ignored and all the files are monitored.
3. ``verbose``: This boolean setting decides whether logs are shown in stdout. Defaults to _true_. 
4. ``logfile_path``: This sets the path for the file where the logs will be writtten in JSON format.

### External libraries used:
- [lwjson](https://github.com/MaJerle/lwjson/tree/master) to parse JSON files.
- [cwalk](https://github.com/likle/cwalk) to parse paths.
