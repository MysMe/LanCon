#pragma once

#ifdef _WIN32
	#define LANCONEXPORT extern "C" __declspec(dllexport)
#else
#error OS not supported
#endif