#include <iostream>
#include <map>

#include "vulkan/vulkan.h"
#include "thesis.h"
#include "logger.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <any>
#include <bitset>
#include <queue>
#include <fstream>
// #include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>
#endif // STB_IMAGE_IMPLEMENTATION

#include <nlohmann/json.hpp>

// #ifndef TINYOBJLOADER_IMPLEMENTATION
// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>
// #endif // TINYOBJLOADER_IMPLEMENTATION

#include <sheriff-base.h>
#include <flagex_base.h>

#include "engine/data/resource_base.hpp"