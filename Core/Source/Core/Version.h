#pragma once

#define VERSION "2025.1.0-dev"

//
// Build Configuration
//
#if defined(DEBUG)
#define BUILD_CONFIG_NAME "Debug"
#elif defined(RELEASE)
#define BUILD_CONFIG_NAME "Release"
#elif defined(DIST)
#define BUILD_CONFIG_NAME "Dist"
#else
#error Undefined configuration?
#endif

//
// Build Platform
//
#if defined(PLATFORM_WINDOWS)
#define BUILD_PLATFORM_NAME "Windows x64"
#elif defined(PLATFORM_LINUX)
#define BUILD_PLATFORM_NAME "Linux"
#else
#define BUILD_PLATFORM_NAME "Unknown"
#endif

#define VERSION_LONG "Engine " VERSION " (" BUILD_PLATFORM_NAME " " BUILD_CONFIG_NAME ")"
