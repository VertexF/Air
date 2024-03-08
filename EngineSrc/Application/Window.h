#ifndef WINDOW_HDR
#define WINDOW_HDR

#include "Foundation/Service.h"
#include "Foundation/Array.h"

namespace Air 
{

    struct WindowConfiguration
    {
        uint32_t width;
        uint32_t height;

        const char* name;

        Allocator* allocator;
    };

    typedef void(*OsMessagesCallback)(void *osEvent, void* userData);

    struct Window : public Service 
    {
        void init(void* config) override;
        void shutdown() override;

        void handlOSMessages();

        void setFullscreen(bool value);

        void registerOSMessagesCallback(OsMessagesCallback callback, void* userData);
        void unredisterOSMessagesCallback(OsMessagesCallback callback);

        void centerMouse(bool dragging);

        Array<OsMessagesCallback> OSMessagesCallbacks;
        Array<void*> OSMessagesCallbacksData;

        void* platformHandle = nullptr;
        bool requestedExit = false;
        bool resized = false;
        bool minimused = false;
        uint32_t width = 0;
        uint32_t height = 0;
        float displayRefresh = 1.f / 60.f;

        static constexpr const char* NAME = "WINDOW_SERVICE";
    };
}

#endif // !WINDOW_HDR
