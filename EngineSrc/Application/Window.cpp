#include "Window.h"

#include "Foundation/Log.h"
#include "Foundation/Numerics.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "vender/imgui/imgui.h"
#include "vender/imgui/imgui_impl_sdl2.h"

namespace 
{
    SDL_Window* window = nullptr;
}//Anon

namespace Air 
{
    static float SDLGetMonitorRefresh() 
    {
        SDL_DisplayMode current;
        int shouldBeZero = SDL_GetCurrentDisplayMode(0, &current);
        AIR_ASSERTM(shouldBeZero == 0, "The value is not equal to zero.");
        return 1.f / current.refresh_rate;
    }

    void Window::init(void* config)
    {
        aprint("Window Serive starting up!\n");

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
        {
            AIR_ASSERTM(false, "SDL Init error : %s\n", SDL_GetError());
            return;
        }

        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);

        WindowConfiguration& configuration = *reinterpret_cast<WindowConfiguration*>(config);
        SDL_WindowFlags windowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        window = SDL_CreateWindow(configuration.name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  configuration.width, configuration.height, windowFlags);
        aprint("Window created successfully.\n");

        int windowWidth, windowHeigth;
        SDL_Vulkan_GetDrawableSize(window, &windowWidth, &windowHeigth);

        width = static_cast<uint32_t>(windowWidth);
        height = static_cast<uint32_t>(windowHeigth);

        //Assing this os it can be accessed from outside.
        platformHandle = window;

        //Callback
        OSMessagesCallbacks.init(configuration.allocator, 4);
        OSMessagesCallbacksData.init(configuration.allocator, 4);

        displayRefresh = SDLGetMonitorRefresh();
    }

    void Window::shutdown() 
    {
        OSMessagesCallbacksData.shutdown();
        OSMessagesCallbacks.shutdown();

        SDL_DestroyWindow(window);
        SDL_Quit();

        aprint("Window shutdown.\n");
    }

    void Window::handlOSMessages() 
    {
        SDL_Event events;
        while (SDL_PollEvent(&events)) 
        {
            ImGui_ImplSDL2_ProcessEvent(&events);
            switch (events.type) 
            {
            case SDL_QUIT:
            {
                requestedExit = true;
                break;
            }

            case SDL_WINDOWEVENT:
            {
                switch (events.window.event) 
                {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        uint32_t newWidth = static_cast<uint32_t>(events.window.data1);
                        uint32_t newHeight = static_cast<uint32_t>(events.window.data2);

                        //Update only if needed.
                        if (newWidth != width || newHeight != height) 
                        {
                            resized = true;
                            width = newWidth;
                            height = newHeight;

                            aprint("Resize to %u, %u\n", width, height);
                        }
                    }
                    break;            
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        aprint("Focus gained\n");
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        aprint("Focus Lost\n");
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        aprint("Maximised\n");
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        aprint("Minimised\n");
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        aprint("Restored\n");
                        break;
                    case SDL_WINDOWEVENT_TAKE_FOCUS:
                        aprint("Take Focus\n");
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        aprint("Exposed\n");
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        requestedExit = true;
                        aprint("Window close event received.\n");
                        break;
                    default:
                        displayRefresh = SDLGetMonitorRefresh();
                        break;
                }
            }
            break;

            }
        }
    }

    void Window::setFullscreen(bool value) 
    {
    }

    void Window::registerOSMessagesCallback(OsMessagesCallback callback, void* userData) 
    {
    }

    void Window::unredisterOSMessagesCallback(OsMessagesCallback callback) 
    {
    }

    void Window::centerMouse(bool dragging) 
    {
    }
}