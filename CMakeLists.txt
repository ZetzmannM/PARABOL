cmake_minimum_required(VERSION 3.10)

project(Vulkan VERSION 0)


#Actual Project
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Parabol/)

#Auto Compile Doc
# taken from https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
find_package(Doxygen QUIET)

if(WIN32)
    set(BUILD_RES_DIR_D ${CMAKE_BINARY_DIR}/Parabol/res)
    set(BUILD_RES_BAT ${CMAKE_SOURCE_DIR}/rsccpy.bat)

    file(TO_NATIVE_PATH ${BUILD_RES_DIR_D} BUILD_RES_DIR)

    configure_file(${BUILD_RES_BAT}.in ${BUILD_RES_BAT} @ONLY)

    add_custom_target( copy_res ALL
        COMMAND ${BUILD_RES_BAT}
        COMMENT "Copying Resources to build path"
)
endif(WIN32)

if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
