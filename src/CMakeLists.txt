add_compile_options(-D LWJSON_IGNORE_USER_OPTS)

# add library
add_library(
    intercept SHARED
    intercept.c
)

add_library(
    lwjson STATIC
    lwjson/lwjson_debug.c
    lwjson/lwjson.c
)

# the utility executables
add_executable(listener listener.c)

target_link_libraries(intercept PRIVATE -ldl)
target_link_libraries(listener PRIVATE lwjson)