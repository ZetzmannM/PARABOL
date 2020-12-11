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
add_executable(Parabol ${FLG} "main.cpp" "ptr.h" "inttypes.h"   "ChannelPrintStream.h" "ChannelPrintStream.cpp" "Env.h" "StringUtils.h" "StringUtils.cpp" "Surface.h" "Surface.cpp" "Timer.cpp" "Timer.h" "VkUtils.cpp" "VkUtils.h" "meta_tags.h" "inc_setting.h")

set_property(TARGET Parabol PROPERTY CXX_STANDARD 17)

if(WIN32)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/glfw3.lib)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/vulkan-1.lib)
endif (WIN32)

if(UNIX)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libglfw3.a)
    target_link_libraries(Parabol ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libvulkan-1.a)
endif (UNIX)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dep_inc)

SOURCE_GROUP("1HeaderFiles" FILES ${LOCAL_SOURCE})
SOURCE_GROUP("2SourceFiles" FILES ${LOCAL_SOURCE_C})
SOURCE_GROUP("3Instance" FILES " ")
