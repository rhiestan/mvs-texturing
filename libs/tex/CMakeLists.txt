file(GLOB SOURCES CONFIGURE_DEPENDS "[^_]*.cpp")

add_library(tex STATIC ${SOURCES})
target_link_libraries(tex PUBLIC tex_options)
install(TARGETS tex ARCHIVE DESTINATION lib)
