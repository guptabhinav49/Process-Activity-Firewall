add_compile_options(-D LWJSON_IGNORE_USER_OPTS)

# add library
add_library(
    lwjson STATIC
    lwjson/lwjson_debug.c
    lwjson/lwjson.c
)
add_library(
    cwalk STATIC
    cwalk/cwalk.c
)
add_library(
    trie STATIC
    trie/trie.c
    trie/vector.c
)
add_library(
    intercept SHARED
    intercept.c
)

# the utility executables
add_executable(listener listener.c utilites.c)

target_link_libraries(intercept PRIVATE -ldl)
target_link_libraries(listener 
    PRIVATE lwjson 
    PRIVATE cwalk
    PRIVATE trie
)