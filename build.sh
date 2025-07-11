#!/bin/bash

# Build script for Windows C++ application
# Usage: ./build.sh [debug|release]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running on Windows or WSL
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
    CXX="cl"
    EXE_EXT=".exe"
elif [[ -n "$WSL_DISTRO_NAME" ]]; then
    PLATFORM="wsl"
    CXX="x86_64-w64-mingw32-g++"
    EXE_EXT=".exe"
else
    print_error "This script is designed for Windows or WSL environment"
    exit 1
fi

# Default build mode
BUILD_MODE="release"

# Parse command line arguments
if [ $# -gt 0 ]; then
    case $1 in
        debug|DEBUG)
            BUILD_MODE="debug"
            ;;
        release|RELEASE)
            BUILD_MODE="release"
            ;;
        *)
            print_error "Invalid build mode: $1"
            echo "Usage: $0 [debug|release]"
            exit 1
            ;;
    esac
fi

print_info "Building in $BUILD_MODE mode on $PLATFORM"

# Project settings
PROJECT_NAME="winshluha"
SOURCE_FILES="main.cpp MainWindow.cpp"
OUTPUT_DIR="build"
EXECUTABLE="$OUTPUT_DIR/$PROJECT_NAME$EXE_EXT"

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Compiler flags based on build mode
if [ "$BUILD_MODE" == "debug" ]; then
    print_info "Debug mode: Enabling debug symbols, console window, and debug overlay"
    
    if [ "$PLATFORM" == "windows" ]; then
        # MSVC flags for debug
        CXXFLAGS="/std:c++17 /EHsc /D_DEBUG /DDEBUG /Zi /Od /MDd"
        LDFLAGS="/DEBUG /SUBSYSTEM:CONSOLE"
        LIBS="user32.lib gdi32.lib d2d1.lib dwrite.lib gdiplus.lib mf.lib mfplat.lib mfreadwrite.lib mfuuid.lib"
    else
        # MinGW flags for debug
        CXXFLAGS="-std=c++17 -g -O0 -D_DEBUG -DDEBUG -mconsole"
        LDFLAGS="-mconsole -static-libgcc -static-libstdc++"
        LIBS="-luser32 -lgdi32 -ld2d1 -ldwrite -lgdiplus -lmf -lmfplat -lmfreadwrite -lmfuuid"
    fi
else
    print_info "Release mode: Optimized build without debug console"
    
    if [ "$PLATFORM" == "windows" ]; then
        # MSVC flags for release
        CXXFLAGS="/std:c++17 /EHsc /O2 /MD /DNDEBUG"
        LDFLAGS="/SUBSYSTEM:WINDOWS"
        LIBS="user32.lib gdi32.lib d2d1.lib dwrite.lib gdiplus.lib mf.lib mfplat.lib mfreadwrite.lib mfuuid.lib"
    else
        # MinGW flags for release
        CXXFLAGS="-std=c++17 -O2 -DNDEBUG -mwindows"
        LDFLAGS="-mwindows -static-libgcc -static-libstdc++"
        LIBS="-luser32 -lgdi32 -ld2d1 -ldwrite -lgdiplus -lmf -lmfplat -lmfreadwrite -lmfuuid"
    fi
fi

# Check if we need to create missing header files
print_info "Checking for missing dependencies..."

if [ ! -f "CameraView.h" ]; then
    print_warning "CameraView.h not found, creating stub file"
    cat > CameraView.h << 'EOF'
#pragma once
#include <d2d1.h>
#include <string>

class CameraView {
public:
    CameraView() = default;
    ~CameraView() = default;
    
    bool Initialize(ID2D1HwndRenderTarget* renderTarget) { return true; }
    void Draw(ID2D1HwndRenderTarget* renderTarget, const D2D1_RECT_F& rect) {}
    void Update() {}
    
    std::string GetResolution() const { return "1920x1080"; }
    int GetFPS() const { return 30; }
    int GetLatency() const { return 16; }
};
EOF
fi

if [ ! -f "AnimatedMenu.h" ]; then
    print_warning "AnimatedMenu.h not found, creating stub file"
    cat > AnimatedMenu.h << 'EOF'
#pragma once
#include <d2d1.h>
#include <dwrite.h>

class AnimatedMenu {
public:
    AnimatedMenu() = default;
    ~AnimatedMenu() = default;
    
    bool Initialize(ID2D1HwndRenderTarget* renderTarget, IDWriteFactory* writeFactory) { return true; }
    void Draw(ID2D1HwndRenderTarget* renderTarget, const D2D1_RECT_F& rect) {}
    void Update() {}
    void Toggle() {}
    void Close() {}
    bool IsOpen() const { return false; }
};
EOF
fi

if [ ! -f "DebugOverlay.h" ]; then
    print_warning "DebugOverlay.h not found, creating stub file"
    cat > DebugOverlay.h << 'EOF'
#pragma once
#include <Windows.h>
#include <string>

class DebugOverlay {
public:
    DebugOverlay() = default;
    ~DebugOverlay() = default;
    
    bool Initialize(HINSTANCE hInstance) { return true; }
    void UpdateCameraStats(const std::string& resolution, int fps, int latency) {}
};
EOF
fi

print_info "Starting compilation..."

# Build the project
if [ "$PLATFORM" == "windows" ]; then
    # Use MSVC compiler
    print_info "Using MSVC compiler"
    if ! command -v cl &> /dev/null; then
        print_error "MSVC compiler not found. Please run this script from Visual Studio Developer Command Prompt"
        exit 1
    fi
    
    cl $CXXFLAGS $SOURCE_FILES /Fe:"$EXECUTABLE" /link $LDFLAGS $LIBS
else
    # Use MinGW compiler
    print_info "Using MinGW compiler"
    if ! command -v $CXX &> /dev/null; then
        print_error "MinGW compiler not found. Please install mingw-w64"
        exit 1
    fi
    
    $CXX $CXXFLAGS $SOURCE_FILES -o "$EXECUTABLE" $LDFLAGS $LIBS
fi

# Check if compilation was successful
if [ $? -eq 0 ]; then
    print_success "Compilation successful!"
    
    # Show build information
    print_info "Build information:"
    echo "  - Mode: $BUILD_MODE"
    echo "  - Platform: $PLATFORM"
    echo "  - Output: $EXECUTABLE"
    echo "  - Size: $(ls -lh "$EXECUTABLE" | awk '{print $5}')"
    
    # Ask if user wants to run the application
    read -p "Do you want to run the application now? (y/n): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "Starting application..."
        if [ "$PLATFORM" == "wsl" ]; then
            # Run through Windows from WSL
            cmd.exe /c "$(wslpath -w "$EXECUTABLE")"
        else
            # Run directly on Windows
            "./$EXECUTABLE"
        fi
    else
        print_info "Application compiled successfully. You can run it with:"
        print_info "  ./$EXECUTABLE"
    fi
else
    print_error "Compilation failed!"
    exit 1
fi
