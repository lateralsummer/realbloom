cmake_minimum_required(VERSION 3.20)
project(RealBloom LANGUAGES CXX C OBJC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress OpenGL deprecation warnings on macOS
if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# --- System dependencies (Homebrew) ---
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(OpenColorIO REQUIRED)
find_package(pugixml REQUIRED)

# OpenMP is optional on macOS (Apple Clang doesn't ship it by default)
find_package(OpenMP)

# --- Vendored: Dear ImGui ---
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/RealBloom/src/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
    # imgui_impl_win32.cpp deliberately excluded on Mac
)

# --- Vendored: pugixml (source) ---
# Using system pugixml via find_package instead of vendored source.
# The vendored headers are still on the include path for #include <pugixml/pugixml.hpp>.

# --- Vendored: NFD Extended ---
set(NFD_DIR ${CMAKE_SOURCE_DIR}/RealBloom/include/nfde)
if(APPLE)
    set(NFD_SOURCES ${NFD_DIR}/nfd_cocoa.m)
else()
    set(NFD_SOURCES ${NFD_DIR}/nfd_win.cpp)
endif()

# --- Main application sources ---
set(SRC ${CMAKE_SOURCE_DIR}/RealBloom/src)

set(REALBLOOM_SOURCES
    ${SRC}/Main.cpp
    ${SRC}/CLI.cpp
    ${SRC}/Config.cpp
    ${SRC}/Async.cpp

    ${SRC}/RealBloom/Convolution.cpp
    ${SRC}/RealBloom/ConvolutionThread.cpp
    ${SRC}/RealBloom/ConvolutionFFT.cpp
    ${SRC}/RealBloom/Dispersion.cpp
    ${SRC}/RealBloom/DispersionThread.cpp
    ${SRC}/RealBloom/Diffraction.cpp
    ${SRC}/RealBloom/GpuHelper.cpp
    ${SRC}/RealBloom/ModuleHelpers.cpp
    ${SRC}/RealBloom/Binary/BinaryData.cpp
    ${SRC}/RealBloom/Binary/BinaryConvFftGpu.cpp
    ${SRC}/RealBloom/Binary/BinaryConvNaiveGpu.cpp
    ${SRC}/RealBloom/Binary/BinaryDispGpu.cpp

    ${SRC}/ColorManagement/CMS.cpp
    ${SRC}/ColorManagement/CmImage.cpp
    ${SRC}/ColorManagement/CmImageIO.cpp
    ${SRC}/ColorManagement/CmXYZ.cpp
    ${SRC}/ColorManagement/CMF.cpp
    ${SRC}/ColorManagement/OcioShader.cpp

    ${SRC}/Utils/Bilinear.cpp
    ${SRC}/Utils/ImageTransform.cpp
    ${SRC}/Utils/Misc.cpp
    ${SRC}/Utils/NumberHelpers.cpp
    ${SRC}/Utils/Random.cpp
    ${SRC}/Utils/Status.cpp
    ${SRC}/Utils/StreamUtils.cpp
    ${SRC}/Utils/StringUtils.cpp
    ${SRC}/Utils/ConsoleColors.cpp
    ${SRC}/Utils/CliParser.cpp
    ${SRC}/Utils/CliStackTimer.cpp
    ${SRC}/Utils/FileDialogs.cpp

    ${SRC}/Utils/OpenGL/GlContext.cpp
    ${SRC}/Utils/OpenGL/GlFrameBuffer.cpp
    ${SRC}/Utils/OpenGL/GlFullPlaneVertices.cpp
    ${SRC}/Utils/OpenGL/GlShaderStorageBuffer.cpp
    ${SRC}/Utils/OpenGL/GlTexture.cpp
    ${SRC}/Utils/OpenGL/GlUtils.cpp
    ${SRC}/Utils/OpenGL/GlVertexArray.cpp
    ${SRC}/Utils/OpenGL/GlVertexBuffer.cpp
)

add_executable(RealBloom
    ${REALBLOOM_SOURCES}
    ${IMGUI_SOURCES}
    ${NFD_SOURCES}
)

target_include_directories(RealBloom PRIVATE
    ${SRC}
    ${CMAKE_SOURCE_DIR}/RealBloom/include
)

target_link_libraries(RealBloom PRIVATE
    glfw
    GLEW::GLEW
    OpenImageIO::OpenImageIO
    OpenColorIO::OpenColorIO
    pugixml::pugixml
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(RealBloom PRIVATE OpenMP::OpenMP_CXX)
endif()

if(APPLE)
    # Cocoa framework for NFD, AppKit for macOS GUI
    target_link_libraries(RealBloom PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework UniformTypeIdentifiers"
    )
endif()

# Copy runtime data next to the binary
add_custom_command(TARGET RealBloom POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/RealBloom/assets
        $<TARGET_FILE_DIR:RealBloom>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/RealBloom/ocio
        $<TARGET_FILE_DIR:RealBloom>/ocio
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/RealBloom/cmf
        $<TARGET_FILE_DIR:RealBloom>/cmf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/RealBloom/config.xml
        $<TARGET_FILE_DIR:RealBloom>/config.xml
)
