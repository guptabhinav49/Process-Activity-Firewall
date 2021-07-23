
add_subdirectory(json)

# add library

add_library(
    cwalk STATIC
    cwalk/cwalk.c
)
add_library(
    intercept SHARED
    intercept.cpp
)
add_library(
    trie STATIC
    trie/trie.cpp
)
# the utility executables
add_executable(listener listener.cpp utilites.cpp)
add_executable(config_test test/config.cpp)

target_link_libraries(intercept 
    PRIVATE -ldl
    PRIVATE cwalk
)

target_link_libraries(listener 
    PRIVATE nlohmann_json::nlohmann_json 
    PRIVATE cwalk
    PRIVATE trie
)
target_compile_options(cwalk
    PRIVATE -fPIC
)
target_link_libraries(config_test 
   PRIVATE trie
   PRIVATE cwalk
   PRIVATE nlohmann_json::nlohmann_json
)
