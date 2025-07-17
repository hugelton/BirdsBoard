cmake_minimum_required(VERSION 3.20)
project(TockusSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt6 automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/tockus_dsp.cpp
    src/pt8211_dac.cpp
    src/coreaudio_engine.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/tockus_dsp.h
    src/pt8211_dac.h
    src/coreaudio_engine.h
)

# Create executable
add_executable(TockusSimulator ${SOURCES} ${HEADERS})

# Link Qt6 libraries and macOS frameworks
target_link_libraries(TockusSimulator Qt6::Core Qt6::Widgets)

# Link macOS CoreAudio frameworks
if(APPLE)
    target_link_libraries(TockusSimulator 
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif()

# Include directories
target_include_directories(TockusSimulator PRIVATE src)

# Compiler flags for audio performance
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(TockusSimulator PRIVATE -O3 -ffast-math)
endif()

# macOS specific settings
if(APPLE)
    set_target_properties(TockusSimulator PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.birdsboards.tockus
        MACOSX_BUNDLE_BUNDLE_VERSION 1.0
        MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    )
endif()