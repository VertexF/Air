#include <iostream>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

int main() 
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Air Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Air";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::cout << "Hello new engine." << std::endl;

    return 0;
}