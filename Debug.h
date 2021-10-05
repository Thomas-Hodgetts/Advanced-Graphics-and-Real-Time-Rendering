#pragma once
#include <string>
#include <Windows.h>

class Debug
{
public:

	static void OutputString(std::string OutputString);
	static void OutputString(int OutputString);
	static void OutputString(float OutputString);
	static void OutputString(double OutputString);

};

