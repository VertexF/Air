#include "ServiceManager.h"
#include "Assert.h"

namespace Air 
{
    static ServiceManager SERVICE_MANAGER;
    ServiceManager* ServiceManager::instance = &SERVICE_MANAGER;

    void ServiceManager::init(Allocator* alloc) 
    {
        aprint("Service Manager starting up.\n");
        allocator = alloc;

        services.init(allocator, 8);
    }

    void ServiceManager::shutdown() 
    {
        services.shutdown();

        aprint("Service Manager shutdown.\n");
    }

    void ServiceManager::addService(Service* service, const char* name) 
    {
        uint64_t hashName = hashCalculate(name);
        FlatHashMapIterator iterator = services.find(hashName);
        AIR_ASSERTM(iterator.isInvalid(), "Overwriting service %s, is this intended?", name);
        services.insert(hashName, service);
    }

    void ServiceManager::removeService(const char* name) 
    {
        uint64_t hashName = hashCalculate(name);
        services.remove(hashName);
    }

    Service* ServiceManager::getService(const char* name) 
    {
        uint64_t hashName = hashCalculate(name);
        return services.get(hashName);
    }
}