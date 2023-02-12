cmake_minimum_required(VERSION 3.7)

project(thesis VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED 20)
set(CXX_STANDARD 20)

#add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/SDL)

include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(PREPROCESSOR_DEFINITIONS
    LHG_LINUX
    LHG_VERBOSE
    LHG_DEBUG
    LHG_PRINT_LVL
    LHG_HIDE_PATH
    #THS_VK_OUTPUT_VERBOSE
    #THS_VK_OUTPUT_INFO
    #THESIS_VULKAN_EXTRA_DEBUG_INFO
    )

macro(make_sdl_locally)
    add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/SDL2)
    set(SDL2_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies/SDL2/include)
    set(SDL2_LIBRARIES SDL2-static)
endmacro()

set(COMPILE_SDL_LOCALLY OFF)

# Includes
if (UNIX)
    find_package(Vulkan REQUIRED) # <-- fix this
    find_library(STB_LIB REQUIRED)
    if (COMPILE_SDL_LOCALLY)
        make_sdl_locally()
    else (COMPILE_SDL_LOCALLY)
        find_package(SDL2 REQUIRED)
    endif (COMPILE_SDL_LOCALLY)
endif (UNIX)
if (WIN32)
    set(Vulkan_PATH C:/VulkanSDK/1.3.224.1)
    set(Vulkan_INCLUDE_DIRS ${Vulkan_PATH}/Include)
    set(Vulkan_LIBRARIES ${Vulkan_PATH}/Lib/vulkan-1.lib)
    if (COMPILE_SDL_LOCALLY)
        make_sdl_locally()
    else (COMPILE_SDL_LOCALLY)
        set(SDL2_INCLUDE_DIRS ${Vulkan_PATH}/Include/SDL2)
        set(SDL2_LIBRARIES ${Vulkan_PATH}/Lib/SDL2.lib)
    endif (COMPILE_SDL_LOCALLY)
endif (WIN32)

set(ABSL_PROPAGATE_CXX_STD ON)
set(DBUILD_TESTING OFF)
set(ABSL_BUILD_TESTING OFF)
set(DABSL_BUILD_TESTING OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/abseil)

# Optional variable check
set(PRINT_ALL_VARS OFF)

macro(print_all_variables) # Stolen from Stackoverflow
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
if (PRINT_ALL_VARS)
    print_all_variables()
endif (PRINT_ALL_VARS)

# Extra compile definitions
add_compile_definitions(${PREPROCESSOR_DEFINITIONS})

# Source files for Thesis
set(SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp
            ${PROJECT_SOURCE_DIR}/src/sdl/sdl_base.cpp
            ${PROJECT_SOURCE_DIR}/src/app/application.cpp
            ${PROJECT_SOURCE_DIR}/src/app/debug_window.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_instance.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_device.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_pipeline.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_commands.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_descriptors.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_images.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_depth.cpp
            # ${PROJECT_SOURCE_DIR}/src/vk/vulkan_model.cpp
            ${PROJECT_SOURCE_DIR}/src/vk/backend/vulkan_device.cpp
            )
set(IMGUI_FILES ${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_demo.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_sdl.cpp
            ${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_vulkan.cpp)

# Create Thesis as executable
add_executable(thesis ${SOURCE} ${IMGUI_FILES})

#target_compile_definitions(thesis PUBLIC ${PREPROCESSOR_DEFINITIONS})
target_include_directories(thesis PUBLIC ${PROJECT_SOURCE_DIR}/dependencies/FlagEx/include
                                ${PROJECT_SOURCE_DIR}/dependencies/Sheriff/include
                                ${PROJECT_SOURCE_DIR}/dependencies/lhg/include
                                ${PROJECT_SOURCE_DIR}/dependencies/stb
                                # C:/stb
                                ${PROJECT_SOURCE_DIR}/dependencies/tinyobjloader
                                ${PROJECT_SOURCE_DIR}/dependencies/imgui
                                ${PROJECT_SOURCE_DIR}/dependencies/imgui/backends
                                ${PROJECT_SOURCE_DIR}/dependencies/nlohmannJson/include
                                # ${PROJECT_SOURCE_DIR}/dependencies/nlohmannJson/single_include
                                #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/include
                                ${SDL2_INCLUDE_DIRS}
                                ${Vulkan_INCLUDE_DIRS}
                                ABSL_COMMON_INCLUDE_DIRS)
target_link_libraries(thesis PUBLIC #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/lib/libvulkan.so
                                ${SDL2_LIBRARIES}
                                ${Vulkan_LIBRARIES}
                                # absl::flat_hash_map
                                absl::flat_hash_set
                                absl::container_common)
target_precompile_headers(thesis PUBLIC ${PROJECT_SOURCE_DIR}/src/pch.h)
# Setup Testing
#add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
