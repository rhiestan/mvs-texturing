file(GLOB SOURCES CONFIGURE_DEPENDS "[^_]*.cpp")

add_executable(texrecon ${SOURCES})
target_link_libraries(texrecon PRIVATE tex)
if(TEX_USE_MIMALLOC)
    target_sources(texrecon PRIVATE ${CMAKE_SOURCE_DIR}/cmake/mimalloc_override.cc)
    target_link_libraries(texrecon PRIVATE ${TEX_MIMALLOC_TARGET})
endif()
install(TARGETS texrecon RUNTIME DESTINATION bin)
