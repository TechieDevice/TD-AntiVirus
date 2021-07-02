#pragma once

#define SIGNSCANNER_API __declspec(dllexport)

namespace Scanner
{
	extern "C" SIGNSCANNER_API void AVstart(PCSTR DBPath, PCSTR Path);
	extern "C" SIGNSCANNER_API void GetNextString(BYTE* byteString);
	//extern "C" SIGNSCANNER_API void Delete();
	//extern "C" SIGNSCANNER_API void Encrypt(BYTE* key, BYTE* iv);
};
