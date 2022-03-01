#include "Debug.h"

//https://www.tutorialspoint.com/how-to-convert-std-string-to-lpcwstr-in-cplusplus
void Debug::OutputString(std::string OutputString)
{
	OutputDebugStringA(OutputString.c_str());
}
void Debug::OutputString(int OutputString)
{
	OutputDebugStringA(std::to_string(OutputString).c_str() + '\n');
}
void Debug::OutputString(float OutputString)
{
	OutputDebugStringA(std::to_string(OutputString).c_str() + '\n');
}
void Debug::OutputString(double OutputString)
{
	OutputDebugStringA(std::to_string(OutputString).c_str() + '\n');
}

void Debug::CreateOutputMessage(std::wstring message)
{
	MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
}
