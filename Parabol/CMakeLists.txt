CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)

FILE(GLOB LOCAL_SOURCE
    "*.hpp"
    "*.h"
)
FILE(GLOB LOCAL_SOURCE_C
    "*.cpp"
    "*.c"
)

# Set Win32 flag for Windows
if(WIN32)
    set(FLG 
    WIN32)
else(WIN32)
    set(FLG 
    )
endif(WIN32)

# Main Executable
add_executable(Parabol ${FLG} "main.cpp"  )

if(WIN32)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/glfw3.lib)
endif (WIN32)

if(UNIX)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libglfw3.a)
endif (UNIX)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dep_inc)

SOURCE_GROUP("HeaderFiles" ${LOCAL_SOURCE})
SOURCE_GROUP("SourceFiles" FILES ${LOCAL_SOURCE_C})
