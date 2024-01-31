#include "Foundation/Log.h"

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

    aprintret("Hello new engine");
    aprintret("How many engines have we made %d", 1);

    return 0;
}