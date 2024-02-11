#ifndef RESOURCE_MANAGER_HDR
#define RESOURCE_MANAGER_HDR

#include "Platform.h"
#include "Assert.h"
#include "HashMap.h"

namespace Air 
{
    struct ResourceManager;

    //Reference counting and named resources.
    struct Resource 
    {
        void addReference() { ++references; }
        void removeReference() { AIR_ASSERT(references != 0); --references; }

        uint64_t references = 0;
        const char* name = nullptr;
    };

    //TODO: Look up what a resource compiler is meant to do.
    struct ResourceCompiler 
    {
        
    };

    //Note: There is a interface class to here in the OG code but I'm not doing it.
    struct ResourceLoader 
    {
        virtual Resource* get(const char* name) = 0;
        virtual Resource* get(uint64_t hashedName)  = 0;
        virtual Resource* unload(const char* name) = 0;
        virtual Resource* createFromFile(const char* name, const char* filename, ResourceManager* resourceManager) 
        {
            return nullptr;
        }
    };

    struct ResourceFilenameResolver 
    {
        virtual const char* getBinaryPathFromPath(const char* name) = 0;
    };

    struct ResourceManager 
    {
        void init(Allocator* alloc, ResourceFilenameResolver* resolver);
        void shutdown();

        template<typename T>
        T* load(const char* name) 
        {
            ResourceLoader* loader = loaders.get(T::HASH_TYPE);
            if (loader) 
            {
                //Search if the resource is already in cache.
                T* resource = (T*)loader->get(name);
                if (resource) 
                {
                    return resource;
                }
            }

            //Resource not in cache, create from file.
            const char* path = filenameResolver->getBinaryPathFromPath(name);
            return (T*)loader->createFromFile(name, path, this);
        }

        template<typename T>
        T* get(const char* name) 
        {
            ResourceLoader* loader = loaders.get(T::HASH_TYPE);
            if (loader) 
            {
                return (T*)loader->get(name);
            }
        }

        template<typename T>
        T* get(uint64_t hashedName) 
        {
            ResourceLoader* loader = loaders.get(T::HASH_TYPE);
            if (loader)
            {
                return (T*)loader->get(hashedName);
            }

            return nullptr;
        }

        template<typename T>
        T* reload(const char* name) 
        {
            ResourceLoader* loader = loaders.get(T::HASH_TYPE);
            if (loader) 
            {
                T* resource = (T*)loader->get(name);
                if (resource) 
                {
                    loader->unload(name);

                    //Resource not in cache we need to create it from file.
                    const char* path = filenameResolver->getBinaryPathFromPath(name);
                    return (T*)loader->createFromFile(name, path, this);
                }
            }

            return nullptr;
        }

        void setLoader(const char* resourceType, ResourceLoader* loader);
        void setCompiler(const char* resourceType, ResourceCompiler* compiler);

        FlatHashMap<uint64_t, ResourceLoader*> loaders;
        FlatHashMap<uint64_t, ResourceCompiler*> compilers;

        Allocator* allocator = nullptr;
        ResourceFilenameResolver* filenameResolver;
    };
}

#endif // !RESOURCE_MANAGER_HDR
