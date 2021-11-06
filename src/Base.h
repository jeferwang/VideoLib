#pragma once
#ifdef UE_GAME
#define VP_DLL_EXPORT
#else
#define VP_DLL_EXPORT __declspec( dllexport )
#endif