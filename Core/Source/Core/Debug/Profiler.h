#pragma once

#define ENABLE_PROFILING !DIST

#if ENABLE_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#if ENABLE_PROFILING
#define PROFILE_MARK_FRAME			FrameMark;
// NOTE(Peter): Use PROFILE_FUNC ONLY at the top of a function
//				Use PROFILE_SCOPE / PROFILE_SCOPE_DYNAMIC for an inner scope
#define PROFILE_FUNC(...)			ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
#define PROFILE_SCOPE(...)			PROFILE_FUNC(__VA_ARGS__)
#define PROFILE_SCOPE_DYNAMIC(NAME)  ZoneScoped; ZoneName(NAME, strlen(NAME))
#define PROFILE_THREAD(...)          tracy::SetThreadName(__VA_ARGS__)
#else
#define PROFILE_MARK_FRAME
#define PROFILE_FUNC(...)
#define PROFILE_SCOPE(...)
#define PROFILE_SCOPE_DYNAMIC(NAME)
#define PROFILE_THREAD(...)
#endif
