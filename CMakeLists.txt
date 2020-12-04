cmake_minimum_required(VERSION 3.10)

project(Vulkan)

add_executable(Test "src/main.cpp")

if(WIN32)
    target_link_libraries(Test ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib)
endif (WIN32)

if(UNIX)
    target_link_libraries(Test ${CMAKE_CURRENT_SOURCE_DIR}/lib/libglfw3.so)
endif (UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep_inc)

# taken from https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
find_package(Doxygen QUIET)

if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
