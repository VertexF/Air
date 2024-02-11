#ifndef SERVICE_MANAGER_HDR
#define SERVICE_MANAGER_HDR

#include "Array.h"
#include "HashMap.h"

namespace Air 
{
    struct Service;

    struct ServiceManager 
    {
        void init(Allocator* alloc);
        void shutdown();

        void addService(Service* service, const char* name);
        void removeService(const char* name);

        Service* getService(const char* name);

        template<typename T>
        T* get() 
        {
            T* service = dynamic_cast<T*>(getService(T::NAME));
            if (service == nullptr) 
            {
                addService(T::instance(), T::NAME);
            }

            return T::instance();
        }

        static ServiceManager* instance;

        FlatHashMap<uint64_t, Service*> services;
        Allocator* allocator = nullptr;
    };
}

#endif // !SERVICE_MANAGER_HDR
