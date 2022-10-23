#include "pch.h"
#include "vk/vulkan-base.h"
#include "logger.h"

int main()
{
    LOG_INFO("Hello World!");
    LOG_DEBUG("Hello World!");
    LOG_WARN("Hello World!");
    LOG_ERROR("Hello World!");
    LOG_CRIT("Hello World!");
    return 0;
}