#include "system.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::string System::s_curDir;
bool System::argSilent = false;
util::ArgumentSet System::args;
static std::string s_outDir;

static std::vector<std::pair<std::string,size_t>> s_warnings;
static std::vector<std::pair<std::string, size_t>> s_errors;
static std::vector<std::pair<std::string, size_t>> s_infos;
static ei::Mat4x4 s_axisSwap = ei::identity4x4();

static 	HANDLE hstdout = nullptr;

void System::init()
{
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

void setConsoleColor(WORD wd)
{
	SetConsoleTextAttribute(hstdout, wd);
}
void setConsoleColorDefault()
{
	SetConsoleTextAttribute(hstdout, 0x07);
}
// returns true if it should be displayed
bool addMessage(std::vector<std::pair<std::string, size_t>>& vec, const std::string& msg)
{
	for(auto& e : vec)
	{
		if(e.first == msg)
		{
			e.second++;
			// only display messages up to 10 times
			return e.second < 10;
		}
	}
	vec.push_back(std::make_pair(msg, 1));
	return true;
}

std::string System::fixPath(std::string s)
{
	// make / to \"
	for (auto& c : s)
		if (c == '/')
			c = '\\';

	// remove double /'s
	size_t p = 0;
	while ((p = s.find("\\\\")) != std::string::npos)
	{
		s = s.substr(0, p) + s.substr(p + 1, s.length() - p - 1);
	}

	// make path\path2\..\file -> path\file

	while ((p = s.find("\\..\\")) != std::string::npos)
	{
		// scroll back till "\"
		size_t end = p + 3;
		if (p > 0)
			p--;
		while (p > 0 && s.at(p) != '\\') p--;
		// cut
		s = s.substr(0, p) + s.substr(end, s.length() - end);
	}

	return s;
}

std::string System::removeFileEnding(std::string s)
{
	auto pos = s.find_last_of('.');
	if (pos != std::string::npos)
		s = s.substr(0, pos);
	return s;
}

std::string System::getFileDirectory(std::string s)
{
	// remove filename if given
	size_t pos = s.find_last_of('\\'); // windows style
	if (pos != std::string::npos)
		s = s.substr(0, pos + 1);
	else
	{
		pos = s.find_last_of('/'); // linux style
		if (pos != std::string::npos)
			s = s.substr(0, pos + 1);
	}
	return s;
}

std::string System::getFilename(std::string s)
{
	s = fixPath(s);
	auto pos = s.find_last_of('\\');
	if (pos != std::string::npos)
		s = s.substr(pos + 1, s.length() - pos - 1);
	return s;
}

void System::warning(const std::string& txt)
{
	setConsoleColor(0x0E);
	if(addMessage(s_warnings, txt) && !argSilent)
		std::cerr << "WARNING: " << txt << std::endl;
	setConsoleColorDefault();
}

void System::info(const std::string& txt)
{
	if(addMessage(s_infos, txt) && !argSilent)
		std::cerr << "INFO: " << txt << std::endl;
}

void System::runtimeInfo(const std::string& txt)
{
	if(!argSilent)
		std::cerr << "INFO: " << txt << std::endl;
}

void System::runtimeInfoSpam(const std::string& txt)
{
	if(!argSilent)
	{
		static auto last = std::chrono::high_resolution_clock::now();
		auto now = std::chrono::high_resolution_clock::now();
		if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() > 200)
		{
			last = now;
			runtimeInfo(txt);
		}
	}
}

void System::error(const std::string& txt)
{
	setConsoleColor(0x0C);
	addMessage(s_errors, txt);
	std::cerr << "ERROR: " << txt << std::endl;
	setConsoleColorDefault();
	if (args.has("errpause"))
		system("pause");
}

void displayStrings(const std::vector<std::pair<std::string,size_t>>& vec, const std::string& name)
{
	size_t count = 0;
	for (const auto& e : vec)
		count += e.second;
	if(count)
	{
		std::cerr << name << " (" << count << "):\n";
		for (const auto& e : vec)
		{
			std::cerr << "(" << e.second << ") " << e.first << std::endl;
		}
	}
}

void System::displayWarnings()
{
	setConsoleColor(0x0E);
	displayStrings(s_warnings, "WARNINGS");
	setConsoleColorDefault();
}

void System::displayErrors()
{
	setConsoleColor(0x0C);
	displayStrings(s_errors, "ERRORS");
	setConsoleColorDefault();
}

void System::displayInfos()
{
	setConsoleColor(0x0A);
	displayStrings(s_infos, "INFOS");
	setConsoleColorDefault();
}

size_t System::getAvailableRam()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(memInfo);
	GlobalMemoryStatusEx(&memInfo);

	return memInfo.ullAvailPhys;
}

void System::setOutputDirectory(const std::string& dir)
{
	s_outDir = getFileDirectory(dir);
	// test if you can save files in the output directory
	// create dummy file
	FILE* tmp = fopen((dir + "tmp").c_str(), "wb");
	if(tmp)
	{
		fclose(tmp);
		remove((dir + "tmp").c_str());
		return;
	}
	error("cannot write in output directory " + dir);
	throw std::exception("unable to write to output directory");
}

std::string System::getOutputDirectory()
{
	return s_outDir;
}

void System::setAxisSwap(int a1, int a2)
{
	assert(a1 >= 0 && a1 <= 2);
	assert(a2 >= 0 && a2 <= 2);
	assert(a1 != a2);

	auto swapMat = ei::identity4x4();
	
	// swap row vectors
	std::swap(swapMat(a1), swapMat(a2));

	s_axisSwap *= swapMat;
}

ei::Mat4x4 System::getAxisSwap()
{
	return s_axisSwap;
}

bool System::hasAxisSwap()
{
	return s_axisSwap != ei::identity4x4();
}
