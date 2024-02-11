cmake_minimum_required(VERSION 3.26.0)

project(AirEngine
        VERSION 0.0.0
        LANGUAGES CXX C
        DESCRIPTION "Graphics Programming for first principles.")
        
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AIR_SOURCE EngineSrc/Graphics/CommandBuffer.cpp
                   EngineSrc/Graphics/CommandBuffer.h
                  #EngineSrc/Graphics/GPUDevice.cpp
                  #EngineSrc/Graphics/GPUDevice.h
                  #EngineSrc/Graphics/GPUEnum.h
                  #EngineSrc/Graphics/GPUProfiler.cpp
                  #EngineSrc/Graphics/GPUProfiler.h
                  #EngineSrc/Graphics/GPUResources.cpp
                  #EngineSrc/Graphics/GPUResources.h
                  #EngineSrc/Graphics/Renderer.cpp
                  #EngineSrc/Graphics/Renderer.h

                  #EngineSrc/Graphics/AirImgui.cpp
                  #EngineSrc/Graphics/AirImgui.h

                   EngineSrc/main.cpp)

add_executable(Air ${AIR_SOURCE})

if (WIN32)
    target_compile_definitions(Air PRIVATE
                               _CRT_SECURE_NO_WARNINGS
                               WIN32_LEAN_AND_MEAN
                               NOMINMAX)
endif()

if(MSVC)
    set(VULKAN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/include")
    set(VENDOR_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/vender")
    #I need to uses a config file that is generated here. So it's not a 
    #mistake it's a binrary directory.
	
    set(VULKAN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vender/vulkan/lib/vulkan-1.lib")
	set(SDL_LIB "${CMAKE_CURRENT_SOURCE_DIR}/binaries/SDL2-2.28.5/lib/x64/SDL2.lib")
	set(SDLMAIN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/binaries/SDL2-2.28.5/lib/x64/SDL2main.lib")

    #Set the location of the engine source.
    set(ENGINE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc")
else()
    find_package(SDL2 REQUIRED)
endif()

add_compile_definitions("$<$<CONFIG:Debug>:AIR_DEBUG>")

target_include_directories(Air PUBLIC  ${VULKAN_INCLUDE_DIR} 
                                       ${VENDOR_INCLUDES}
                                       ${ENGINE_INCLUDE})

set(AIR_FOUNDATION_SOURCE EngineSrc/Foundation/Array.h
                          EngineSrc/Foundation/Array.cpp
                          EngineSrc/Foundation/Assert.h
                          EngineSrc/Foundation/Bit.cpp
                          EngineSrc/Foundation/Bit.h
                          EngineSrc/Foundation/BlobSerialisation.cpp
                          EngineSrc/Foundation/BlobSerialisation.h
                          EngineSrc/Foundation/Blob.h
                          # EngineSrc/Foundation/Camera.cpp
                          # EngineSrc/Foundation/Camera.h
                          EngineSrc/Foundation/Colour.cpp
                          EngineSrc/Foundation/Colour.h
                          EngineSrc/Foundation/DataStructures.cpp
                          EngineSrc/Foundation/DataStructures.h
                          EngineSrc/Foundation/File.cpp
                          EngineSrc/Foundation/File.h
                          # EngineSrc/Foundation/Gltf.cpp
                          # EngineSrc/Foundation/Gltf.h
                          EngineSrc/Foundation/HashMap.h
                          EngineSrc/Foundation/HashMap.cpp
                          EngineSrc/Foundation/Log.cpp
                          EngineSrc/Foundation/Log.h
                          EngineSrc/Foundation/MemoryUtils.h
                          EngineSrc/Foundation/Memory.cpp
                          EngineSrc/Foundation/Memory.h
                          EngineSrc/Foundation/Numerics.cpp
                          EngineSrc/Foundation/Numerics.h
                          EngineSrc/Foundation/Platform.h
                          EngineSrc/Foundation/Process.cpp
                          EngineSrc/Foundation/Process.h
                          EngineSrc/Foundation/RelativeDataStructures.h
                          EngineSrc/Foundation/RelativeDataStructures.cpp
                          EngineSrc/Foundation/ResourceManager.cpp
                          EngineSrc/Foundation/ResourceManager.h
                          # EngineSrc/Foundation/Serialization.cpp
                          # EngineSrc/Foundation/Serialization.h
                          EngineSrc/Foundation/ServiceManager.cpp
                          EngineSrc/Foundation/ServiceManager.h
                          EngineSrc/Foundation/Service.h
                          EngineSrc/Foundation/String.cpp
                          EngineSrc/Foundation/String.h
                          EngineSrc/Foundation/Time.cpp
                          EngineSrc/Foundation/Time.h
                          EngineSrc/Foundation/WindowsDeclarations.h
)
add_library(AirFoundation STATIC ${AIR_FOUNDATION_SOURCE})

#set_property(TARGET AirFoundation PROPERTY CXX_STANDARD 17)

target_compile_definitions(AirFoundation PRIVATE
    _CRT_SECURE_NO_WARNINGS

    TRACY_ENABLE
    TRACY_ON_DEMAND
    TRACY_NO_SYSTEM_TRACING
)

target_include_directories(AirFoundation PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${ENGINE_INCLUDE}
                           ${VENDOR_INCLUDES}
)

set(AIR_APPLICATION_SOURCE EngineSrc/Application/Application.cpp
                           EngineSrc/Application/Application.h
                           # EngineSrc/Application/GameCamera.cpp
                           # EngineSrc/Application/GameCamera.h
                           # EngineSrc/Application/GameApplication.cpp
                           # EngineSrc/Application/GameApplication.h
                           # EngineSrc/Application/Input.cpp
                           # EngineSrc/Application/Input.h
                           # EngineSrc/Application/Keys.cpp
                           # EngineSrc/Application/Keys.h
                           # EngineSrc/Application/Window.cpp
                           # EngineSrc/Application/Window.h
)

add_library(AirApplication STATIC ${AIR_APPLICATION_SOURCE})

#set_property(TARGET AirApplication PROPERTY CXX_STANDARD 17)

target_compile_definitions(AirApplication PRIVATE
    _CRT_SECURE_NO_WARNINGS

    TRACY_ENABLE
    TRACY_ON_DEMAND
    TRACY_NO_SYSTEM_TRACING
)

target_include_directories(AirApplication PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/EngineSrc
)

if (WIN32)
    target_include_directories(AirApplication PRIVATE
                               binaries/SDL2-2.28.5/include)
else()
    target_include_directories(AirApplication PRIVATE
        ${SDL2_INCLUDE_DIRS})
endif()

add_library(AirExternal STATIC
    vender/tlsf.c

    vender/json.hpp

    vender/imgui/imconfig.h
    vender/imgui/imgui_draw.cpp
    vender/imgui/imgui_impl_sdl2.cpp
    vender/imgui/imgui_impl_sdl2.h
	vender/imgui/imgui_impl_sdlrenderer2.h
	vender/imgui/imgui_impl_sdlrenderer2.cpp
    vender/imgui/imgui_internal.h
    vender/imgui/imgui_memory_editor.h
    vender/imgui/imgui_tables.cpp
    vender/imgui/imgui_widgets.cpp
    vender/imgui/imgui.cpp
    vender/imgui/imgui.h
    vender/imgui/imstb_rectpack.h
    vender/imgui/imstb_textedit.h
    vender/imgui/imstb_truetype.h
    vender/stb_image.h
    vender/imgui/TextEditor.cpp
    vender/imgui/TextEditor.h

    vender/tracy/tracy/Tracy.hpp
    vender/tracy/tracy/TracyVulkan.hpp
    vender/tracy/TracyClient.cpp

    vender/enkiTS/LockLessMultiReadPipe.h
    vender/enkiTS/TaskScheduler.cpp
    vender/enkiTS/TaskScheduler.h

    vender/meshoptimizer/allocator.cpp
    vender/meshoptimizer/clusterizer.cpp
    vender/meshoptimizer/indexcodec.cpp
    vender/meshoptimizer/indexgenerator.cpp
    vender/meshoptimizer/meshoptimizer.h
    vender/meshoptimizer/overdrawanalyzer.cpp
    vender/meshoptimizer/overdrawoptimizer.cpp
    vender/meshoptimizer/simplifier.cpp
    vender/meshoptimizer/spatialorder.cpp
    vender/meshoptimizer/stripifier.cpp
    vender/meshoptimizer/vcacheanalyzer.cpp
    vender/meshoptimizer/vcacheoptimizer.cpp
    vender/meshoptimizer/vertexcodec.cpp
    vender/meshoptimizer/vertexfilter.cpp
    vender/meshoptimizer/vfetchanalyzer.cpp
    vender/meshoptimizer/vfetchoptimizer.cpp
)

if (WIN32)
    target_sources(AirExternal PRIVATE
                   vender/StackWalker.cpp)
endif()

target_compile_definitions(AirExternal PRIVATE
    TRACY_ENABLE
    TRACY_ON_DEMAND
    TRACY_NO_SYSTEM_TRACING
)

if (WIN32)
    target_include_directories(AirExternal PRIVATE
                               binaries/SDL2-2.28.5/include
                               EngineSrc/
    )
else()
    target_link_libraries(AirExternal PRIVATE
                          dl
    )
    target_include_directories(AirExternal PRIVATE
                               ${SDL2_INCLUDE_DIRS})
endif()

if (WIN32)
    target_link_libraries(Air PRIVATE
                          ${SDL_LIB} ${SDLMAIN_LIB})
else()
    target_link_libraries(Air PRIVATE
                              dl
                              pthread
                              SDL2::SDL2)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${AIR_FOUNDATION_SOURCE} ${AIR_SOURCE} ${AIR_APPLICATION_SOURCE})

target_link_libraries(Air PRIVATE ${VULKAN_LIB} AirFoundation AirApplication AirExternal)

target_compile_options(Air PRIVATE
                       $<$<CXX_COMPILER_ID:MSVC>:/W4>
                       $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Air)
endif()

if (WIN32)
    set(DLLS_TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/binaries/SDL2-2.28.5/lib/x64/SDL2.dll")

    foreach(DLL ${DLLS_TO_COPY})
        add_custom_command(TARGET Air POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DLL} "$(OutDir)"
            VERBATIM
        )
    endforeach()
endif()


