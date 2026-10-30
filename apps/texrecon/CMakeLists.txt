file(GLOB SOURCES CONFIGURE_DEPENDS "[^_]*.cpp")

add_executable(texrecon ${SOURCES})
target_link_libraries(texrecon PRIVATE tex)
install(TARGETS texrecon RUNTIME DESTINATION bin)
