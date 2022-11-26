#include <iostream>

#include "vulkan/vulkan.h"
#include "thesis.h"
#include "logger.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>
#endif // STB_IMAGE_IMPLEMENTATION

#include <sheriff-base.h>
#include <flagex_base.h>