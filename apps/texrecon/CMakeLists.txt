file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cpp")

set(BIN texrecon)

add_executable(${BIN} ${SOURCES})
add_dependencies(${BIN} ext_mve)
target_link_libraries(${BIN} tex -lmve -lmve_util)
