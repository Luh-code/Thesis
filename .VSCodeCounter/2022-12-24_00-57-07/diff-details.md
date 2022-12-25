# Diff Details

Date : 2022-12-24 00:57:07

Directory d:\\Projects\\Thesis

Total : 48 files,  109 codes, 183 comments, 37 blanks, all 329 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [README.md](/README.md) | Markdown | 1 | 0 | 1 | 2 |
| [src/app/application.cpp](/src/app/application.cpp) | C++ | 229 | 21 | 41 | 291 |
| [src/app/application.h](/src/app/application.h) | C++ | 86 | 5 | 11 | 102 |
| [src/ecs/ecs_base.hpp](/src/ecs/ecs_base.hpp) | C++ | 343 | 9 | 74 | 426 |
| [src/ecs/rendering_ecs.hpp](/src/ecs/rendering_ecs.hpp) | C++ | 76 | 0 | 13 | 89 |
| [src/main.cpp](/src/main.cpp) | C++ | 44 | 10 | 7 | 61 |
| [src/memory_mger.h](/src/memory_mger.h) | C++ | 6 | 26 | 9 | 41 |
| [src/pch.h](/src/pch.h) | C++ | 31 | 5 | 5 | 41 |
| [src/sdl/sdl_base.cpp](/src/sdl/sdl_base.cpp) | C++ | 135 | 0 | 15 | 150 |
| [src/sdl/sdl_base.h](/src/sdl/sdl_base.h) | C++ | 22 | 0 | 4 | 26 |
| [src/thesis.h](/src/thesis.h) | C++ | 6 | 0 | 3 | 9 |
| [src/vk/shaders/compile_shaders.bat](/src/vk/shaders/compile_shaders.bat) | Batch | 3 | 0 | 0 | 3 |
| [src/vk/shaders/triangle.frag](/src/vk/shaders/triangle.frag) | GLSL | 9 | 1 | 4 | 14 |
| [src/vk/shaders/triangle.vert](/src/vk/shaders/triangle.vert) | GLSL | 17 | 0 | 4 | 21 |
| [src/vk/vulkan_base.h](/src/vk/vulkan_base.h) | C++ | 313 | 14 | 65 | 392 |
| [src/vk/vulkan_commands.cpp](/src/vk/vulkan_commands.cpp) | C++ | 214 | 4 | 27 | 245 |
| [src/vk/vulkan_depth.cpp](/src/vk/vulkan_depth.cpp) | C++ | 50 | 0 | 8 | 58 |
| [src/vk/vulkan_descriptors.cpp](/src/vk/vulkan_descriptors.cpp) | C++ | 122 | 1 | 22 | 145 |
| [src/vk/vulkan_device.cpp](/src/vk/vulkan_device.cpp) | C++ | 352 | 5 | 65 | 422 |
| [src/vk/vulkan_images.cpp](/src/vk/vulkan_images.cpp) | C++ | 324 | 12 | 33 | 369 |
| [src/vk/vulkan_instance.cpp](/src/vk/vulkan_instance.cpp) | C++ | 195 | 19 | 17 | 231 |
| [src/vk/vulkan_model.cpp](/src/vk/vulkan_model.cpp) | C++ | 54 | 9 | 7 | 70 |
| [src/vk/vulkan_pipeline.cpp](/src/vk/vulkan_pipeline.cpp) | C++ | 400 | 168 | 74 | 642 |
| [tests/test1/Test.cpp](/tests/test1/Test.cpp) | C++ | 5 | 0 | 1 | 6 |
| [d:\dev\Thesis\README.md](/d:%5Cdev%5CThesis%5CREADME.md) | Markdown | -1 | 0 | -1 | -2 |
| [d:\dev\Thesis\src\app\application.cpp](/d:%5Cdev%5CThesis%5Csrc%5Capp%5Capplication.cpp) | C++ | -230 | -7 | -36 | -273 |
| [d:\dev\Thesis\src\app\application.h](/d:%5Cdev%5CThesis%5Csrc%5Capp%5Capplication.h) | C++ | -78 | -5 | -11 | -94 |
| [d:\dev\Thesis\src\ecs\ecs_base.hpp](/d:%5Cdev%5CThesis%5Csrc%5Cecs%5Cecs_base.hpp) | C++ | -347 | -2 | -74 | -423 |
| [d:\dev\Thesis\src\ecs\ecs_base_old.h](/d:%5Cdev%5CThesis%5Csrc%5Cecs%5Cecs_base_old.h) | C++ | -128 | -7 | -24 | -159 |
| [d:\dev\Thesis\src\main.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cmain.cpp) | C++ | -19 | -8 | -10 | -37 |
| [d:\dev\Thesis\src\memory_mger.h](/d:%5Cdev%5CThesis%5Csrc%5Cmemory_mger.h) | C++ | -6 | -26 | -9 | -41 |
| [d:\dev\Thesis\src\pch.h](/d:%5Cdev%5CThesis%5Csrc%5Cpch.h) | C++ | -31 | -5 | -5 | -41 |
| [d:\dev\Thesis\src\sdl\sdl_base.cpp](/d:%5Cdev%5CThesis%5Csrc%5Csdl%5Csdl_base.cpp) | C++ | -135 | 0 | -15 | -150 |
| [d:\dev\Thesis\src\sdl\sdl_base.h](/d:%5Cdev%5CThesis%5Csrc%5Csdl%5Csdl_base.h) | C++ | -22 | 0 | -4 | -26 |
| [d:\dev\Thesis\src\thesis.h](/d:%5Cdev%5CThesis%5Csrc%5Cthesis.h) | C++ | -6 | 0 | -3 | -9 |
| [d:\dev\Thesis\src\vk\shaders\compile_shaders.bat](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cshaders%5Ccompile_shaders.bat) | Batch | -3 | 0 | 0 | -3 |
| [d:\dev\Thesis\src\vk\shaders\triangle.frag](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cshaders%5Ctriangle.frag) | GLSL | -9 | -1 | -4 | -14 |
| [d:\dev\Thesis\src\vk\shaders\triangle.vert](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cshaders%5Ctriangle.vert) | GLSL | -17 | 0 | -4 | -21 |
| [d:\dev\Thesis\src\vk\vulkan_base.h](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_base.h) | C++ | -242 | -11 | -46 | -299 |
| [d:\dev\Thesis\src\vk\vulkan_commands.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_commands.cpp) | C++ | -165 | -1 | -25 | -191 |
| [d:\dev\Thesis\src\vk\vulkan_depth.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_depth.cpp) | C++ | -50 | 0 | -8 | -58 |
| [d:\dev\Thesis\src\vk\vulkan_descriptors.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_descriptors.cpp) | C++ | -122 | -1 | -22 | -145 |
| [d:\dev\Thesis\src\vk\vulkan_device.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_device.cpp) | C++ | -353 | -3 | -65 | -421 |
| [d:\dev\Thesis\src\vk\vulkan_images.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_images.cpp) | C++ | -324 | -11 | -34 | -369 |
| [d:\dev\Thesis\src\vk\vulkan_instance.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_instance.cpp) | C++ | -195 | -19 | -17 | -231 |
| [d:\dev\Thesis\src\vk\vulkan_model.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_model.cpp) | C++ | -52 | -7 | -7 | -66 |
| [d:\dev\Thesis\src\vk\vulkan_pipeline.cpp](/d:%5Cdev%5CThesis%5Csrc%5Cvk%5Cvulkan_pipeline.cpp) | C++ | -388 | -12 | -48 | -448 |
| [d:\dev\Thesis\tests\test1\Test.cpp](/d:%5Cdev%5CThesis%5Ctests%5Ctest1%5CTest.cpp) | C++ | -5 | 0 | -1 | -6 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details