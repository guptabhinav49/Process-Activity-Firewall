# Process-Activity-Firewall

### Steps to use the library:

1. Clone the repository and make sure you have CMake installed (version 3.10 or above).
2. Make a dir _build_ in the root directory and navigate into it using ``mkdir build && cd build``. Then finally issue ``cmake ../ && cmake --build .`` to compile the executables.
3. Load the library by issuing the command: ``export LD_PRELOAD=$PWD/src/libintercept.so``.
4. Run ``./src/listener`` in another shell, all the intercept logs will be displayed here.
5. To use the _substring_ feature of the listener, pass the substring to match as an argument to the listener executable as an command line argument. For example, ``./src/listener`` prints all the logs while ``./src/listener open`` prints logs that contain the substring _open_.

### Config details:

We have four config settings currently that can be set according the user's preference. An example ``config.json`` as well as ``logs.json`` is given in the root directory.

1. ``filegroups``: This is a list of file groups. Each file group has a ``expr``, ``mode`` and ``type`` attributes. The firewall will monitor (or not) the files names given in ``expr`` depending on the ``mode`` attribute. The ``type`` attribute decides how the expressions are matched (for example, complete match or matching the regular expression with current file's absolute path). 
    - ``permission`` - Set this to allow/deny/log the access of the matched files in this group (matching criteria as well as the mode is decided by the other parameters). This parameter is a list, and a list item can be set the following three values
        - _"allow"_
        - _"deny"_
        - _"log_
    A combination of _allow_ with _log_ can also be given in the list. 
    - ``type`` - This sets how the string in the file group is compared to the absolute path the current file being monitored.
        - 0 - _exact match_
        - 10 - _filepaths matching the regular expressions_
        - 11 - _filenames matching the regular expressions_
        - 20 - _filepaths starting with the given string_
        - 21 - _filenames starting with the given string_
        - 30 - _filepaths ending with the given string_
        - 31 - _filenames ending with the given string_
    - ``expr`` - This is a list of strings (or regular expressions depending on the ``type`` attribute)
    - ``mode`` - This sets whether the current group should be ignored or, whether a matched file is shown or not shown.
        - 0 - _none_
        - 1 - _all from list_
        - 2 - _all except from list_

    Note that the regular expressions should be in POSIX (extended) format to work correctly. Also, if a file(name/path) matches 2 groups with different ``(type, mode)``, then the precedence is given to the exact match i.e. if there is some group with type _0_ and some expression in its ``exprs`` mathces then that group's ``mode`` is considered, while if there is no exact match then the first regular expression that matches (if any) decides the mode.

2. ``ignore_filegroups`` : If set ``true``, all filegroups are ignored and all the files are monitored.
3. ``verbose``: This boolean setting decides whether logs are shown in stdout. Defaults to _true_. 
4. ``logfile_path``: This sets the path for the file where the logs will be writtten in JSON format.

### External libraries used:
- [json](https://github.com/nlohmann/json) by nlohmann to parse JSON files.
- [cwalk](https://github.com/likle/cwalk) to parse paths.
- Vector implementation from [here](https://www.sanfoundry.com/c-program-implement-vector/).
- Trie implementation based on _C++_ implementation given [here](https://cp-algorithms.com/string/aho_corasick.html).
