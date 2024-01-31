#ifndef SERVICE_HDR
#define SERVICE_HDR

namespace Air
{
    struct Service
    {
        virtual void init(void* configure) {};
        virtual void shutdown() {};
    };

#define AIR_DECLARE_SERVICE(Type) static Type* instance();

}//AIR

#endif // !SERVICE_HDR
