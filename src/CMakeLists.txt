add_compile_options(-D LWJSON_IGNORE_USER_OPTS)

add_subdirectory(json)

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
    intercept SHARED
    intercept.cpp
)

# the utility executables
add_executable(listener listener.cpp utilites.cpp)
add_executable(config_test test/config.cpp)

target_link_libraries(intercept PRIVATE -ldl)
target_link_libraries(listener 
    PRIVATE nlohmann_json::nlohmann_json 
    PRIVATE cwalk
    PRIVATE trie
)

target_link_libraries(config_test PRIVATE nlohmann_json::nlohmann_json)