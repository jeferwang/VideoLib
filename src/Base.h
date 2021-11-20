#pragma once
// #ifndef UE_GAME
//     #if BUILD_XVIDEO == 1
//     #define XVIDEO_API __declspec( dllexport )
//     #else
//     #define XVIDEO_API __declspec( dllimport )
//     #endif
//
//     #if BUILD_XLOG == 1
//     #define LOG_API __declspec( dllexport )
//     #else
//     #define LOG_API __declspec( dllimport )
//     #endif
//
//     #if BUILD_XGRAPHIC == 1
//     #define D3D_UTILS_API __declspec( dllexport )
//     #else
//     #define D3D_UTILS_API __declspec( dllimport )
//     #endif
// #else
//     #define XVIDEO_API
//     #define LOG_API
//     #define D3D_UTILS_API
// #endif

#if BUILD_XVIDEO == 1
#define XVIDEO_API __declspec( dllexport )
#else
#define XVIDEO_API __declspec( dllimport )
#endif

#if BUILD_XLOG == 1
#define LOG_API __declspec( dllexport )
#else
#define LOG_API __declspec( dllimport )
#endif

#if BUILD_XGRAPHIC == 1
#define D3D_UTILS_API __declspec( dllexport )
#else
#define D3D_UTILS_API __declspec( dllimport )
#endif