set(CMAKE_CXX_STANDARD 20)

include(CTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

set(PREPROCESSOR_DEFINITIONS EZV_SUPPORT_SDL LHG_VERBOSE EZV_INFO_OUTPUT)

add_compile_definitions(${PREPROCESSOR_DEFINITIONS})

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/dependencies/FlagEx/include
                    ${PROJECT_SOURCE_DIR}/dependencies/Sheriff/include
                    ${PROJECT_SOURCE_DIR}/dependencies/lhg/include
                    #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/include
                    ${Vulkan_INCLUDE_DIRS}
                    ${SDL2_INCLUDE_DIRS})
set(COMMON_LIBS #${PROJECT_SOURCE_DIR}/bin/lib/libTVk.a
                #${PROJECT_SOURCE_DIR}/dependencies/Vulkan/1.3.216.0/x86_64/lib/libvulkan.so
                ${Vulkan_LIBRARIES}
                ${SDL2_LIBRARIES})

#enable_testing()

#add_executable(test1 test1/Test.cpp)
#target_include_directories(test1 PUBLIC ${COMMON_INCLUDES})
#target_link_libraries(test1 PUBLIC ${COMMON_LIBS})
#add_test(Test1 test1)