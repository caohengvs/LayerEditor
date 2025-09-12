#pragma once

#ifdef _HF_FECOMMONLAYER_DLL_
#define _HF_FECOMMONLAYER_DLL_HEADER_ __declspec(dllexport)
#define _HF_FECOMMONLAYER_DLL_FUN_HEADER_ extern "C" __declspec(dllexport)
#else
#define _HF_FECOMMONLAYER_DLL_HEADER_ __declspec(dllimport)
#define _HF_FECOMMONLAYER_DLL_FUN_HEADER_ extern "C" __declspec(dllimport)
#endif
