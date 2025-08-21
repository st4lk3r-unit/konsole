cmake_minimum_required(VERSION 3.13)
project(konsole C)

add_library(konsole
    konsole.c
    line.c
    cmd.c
    third_party/npf.c
)

target_include_directories(konsole PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(konsole PUBLIC
    $<$<BOOL:${KONSOLE_ENABLE_VT100}>:KONSOLE_ENABLE_VT100=${KONSOLE_ENABLE_VT100}>
    $<$<BOOL:${KONSOLE_HISTORY}>:KONSOLE_HISTORY=${KONSOLE_HISTORY}>
    $<$<BOOL:${KONSOLE_MAX_LINE}>:KONSOLE_MAX_LINE=${KONSOLE_MAX_LINE}>
    $<$<BOOL:${KONSOLE_MAX_ARGS}>:KONSOLE_MAX_ARGS=${KONSOLE_MAX_ARGS}>
    $<$<BOOL:${KONSOLE_USE_NPF}>:KONSOLE_USE_NPF=${KONSOLE_USE_NPF}>
    $<$<BOOL:${KONSOLE_EOL_MODE}>:KONSOLE_EOL_MODE=${KONSOLE_EOL_MODE}>
)

set_property(TARGET konsole PROPERTY C_STANDARD 99)
