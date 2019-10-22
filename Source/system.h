#pragma once
#include <string>
#include <ei/vector.hpp>
#include "ArgumentSet.h"

// static system class 

class System
{
public:
	static void init();
	// like directory/
	static void setDirectory(const std::string& dir)
	{
		static bool firstTime = true;
		if(args.get("dirhierarchy", false) || firstTime)
			s_curDir = dir;
		firstTime = false;
	}
	static std::string getCurrentDirectory()
	{
		return s_curDir;
	}
	static std::string fixPath(std::string s);
	static std::string removeFileEnding(std::string s);
	static std::string getFileDirectory(std::string s);
	static std::string getFilename(std::string s);
	static void warning(const std::string& txt);
	// information that will be saved (like "bla" not supported)
	static void info(const std::string& txt);
	// information that wont be saved (like loading scene ...)
	static void runtimeInfo(const std::string& txt);
	// information that can spam the console (shape 200/1000) this will be capped at 1 information / second
	static void runtimeInfoSpam(const std::string& txt);
	static void error(const std::string& txt);
	static void displayWarnings();
	static void displayErrors();
	static void displayInfos();
	static size_t getAvailableRam();
	static void setOutputDirectory(const std::string& dir);
	static std::string getOutputDirectory();
	// define which axis to swap 0 = x, 1 = y, 2 = z
	static void setAxisSwap(int a1, int a2);
	static ei::Mat4x4 getAxisSwap();
	static bool hasAxisSwap();
public:
	static bool argSilent;
	static util::ArgumentSet args;
private:
	static std::string s_curDir;
};
