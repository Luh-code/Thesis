cmake_minimum_required(VERSION 3.7)

project(thesis VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

#add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/SDL)

include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(PREPROCESSOR_DEFINITIONS LHG_VERBOSE LHG_LINUX LHG_PRINT_LVL LHG_HIDE_PATH)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_compile_definitions(${PREPROCESSOR_DEFINITIONS})
add_compile_options("-O2")

set(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp
            ${PROJECT_SOURCE_DIR}/src/sdl/sdl-base.cpp
            ${PROJECT_SOURCE_DIR}/src/app/application.cpp)

# Create Thesis as executable
add_executable(thesis ${SOURCE})
#target_compile_definitions(thesis PUBLIC ${PREPROCESSOR_DEFINITIONS})
target_include_directories(thesis PUBLIC ${PROJECT_SOURCE_DIR}/dependencies/FlagEx/include
                                ${PROJECT_SOURCE_DIR}/dependencies/Sheriff/include
                                ${PROJECT_SOURCE_DIR}/dependencies/lhg/include
                                #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/include
                                ${SDL2_INCLUDE_DIRS}
                                ${Vulkan_INCLUDE_DIRS})
target_link_libraries(thesis PUBLIC #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/lib/libvulkan.so
                                ${SDL2_LIBRARIES}
                                ${Vulkan_LIBRARIES})
#target_precompile_headers(thesis PUBLIC ${PROJECT_SOURCE_DIR}/src/pch.h)
# Setup Testing
#add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
