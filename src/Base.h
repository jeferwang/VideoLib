#pragma once
#ifndef UE_GAME

#if BUILD_VP==1
#define VP_API __declspec( dllexport )
#else
#define VP_API __declspec( dllimport )
#endif

#if BUILD_LOGGER==1
#define LOG_API __declspec( dllexport )
#else
#define LOG_API __declspec( dllimport )
#endif

#if BUILD_D3D_UTILS==1
#define D3D_UTILS_API __declspec( dllexport )
#else
#define D3D_UTILS_API __declspec( dllimport )
#endif

#else

#define VP_API
#define LOG_API
#define D3D_UTILS_API

#endif
