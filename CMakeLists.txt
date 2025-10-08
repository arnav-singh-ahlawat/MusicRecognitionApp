# ---- Project Setup ----
# Require CMake version 3.21 or newer
cmake_minimum_required(VERSION 3.21)

# Define project name and primary language
project(MusicRecognitionApp LANGUAGES CXX)

# ---- Build Options ----
# Option to enable/disable GPU acceleration via OpenCL.
# Default: OFF (use CPU-only execution).
option(USE_OPENCL "Enable OpenCL acceleration" OFF)

# ---- Qt Build Settings ----
# Enable automatic handling of MOC, RCC, and UIC (Qt's meta-object tools).
# This simplifies building projects that use Qt's signals/slots and UI system.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- Build Type ----
# Default to Release build if not specified by the user.
# Available options: Debug, Release, RelWithDebInfo, MinSizeRel
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---- Compiler Settings ----
# Enforce modern C++ standard (C++17 required).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Uncomment the line below if you want to disable Qt's built-in keywords
# (signals, slots, emit) and avoid potential naming conflicts.
# add_compile_definitions(QT_NO_KEYWORDS)

# ---- Qt Dependencies ----
# Require Qt6 version 6.5 or newer.
# Core: Base Qt functionality
# Widgets: GUI components
# Multimedia: Audio capture/playback
# Sql: Database (QSqlDatabase) support
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia Sql)

# ---- Source Files ----
# Define all source/header/UI files for the project.
set(SRC
        src/main.cpp

        # ---- UI Layer ----
        src/ui/MainWindow.h src/ui/MainWindow.cpp src/ui/MainWindow.ui
        src/ui/MetadataDialog.h src/ui/MetadataDialog.cpp src/ui/MetadataDialog.ui

        # ---- Audio Layer ----
        src/audio/AudioCapture.h src/audio/AudioCapture.cpp
        src/audio/AudioPlayer.h src/audio/AudioPlayer.cpp
        src/audio/WavFile.h src/audio/WavFile.cpp

        # ---- Database Layer ----
        src/db/Database.h src/db/Database.cpp

        # ---- Fingerprinting (DSP) ----
        src/fingerprint/Fingerprint.h src/fingerprint/Fingerprint.cpp
        src/fingerprint/FFT.h src/fingerprint/FFT.cpp

        # ---- OpenCL Acceleration (Optional) ----
        src/opencl/OpenCLAccel.h src/opencl/OpenCLAccel.cpp
)

# Define the main executable target
add_executable(MusicRecognitionApp ${SRC})

# Include headers from the "src" directory
target_include_directories(MusicRecognitionApp PRIVATE src)

# ---- Link Qt Libraries ----
target_link_libraries(MusicRecognitionApp PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Sql
)

# ---- Optional OpenCL Acceleration ----
if(USE_OPENCL)
    # Link against OpenCL if GPU acceleration is enabled
    target_link_libraries(MusicRecognitionApp PRIVATE OpenCL::OpenCL)
    target_compile_definitions(MusicRecognitionApp PRIVATE USE_OPENCL=1)
else()
    # Fallback: CPU-only execution
    target_compile_definitions(MusicRecognitionApp PRIVATE USE_OPENCL=0)
endif()

# ---- Release Build Optimizations ----
# Apply high optimization (-O3) and disable debug macros (NDEBUG) in Release mode.
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(MusicRecognitionApp PRIVATE -O3 -DNDEBUG)
endif()