#pragma once
#include <memory>
#include "PBRT/PbrtScene.h"
#include "file.h"

// directory like: "mySceneDirectory/"
void parse(const std::unique_ptr<char[]>& data, size_t length, PbrtScene& scene, std::string directory, const std::string& filename);

inline bool parseFile(std::string filename, PbrtScene& scene, std::string curDirectory)
{
	curDirectory = System::fixPath(curDirectory);
	if(curDirectory.size())
	{
		if(!(curDirectory.back() == '\\') || !(curDirectory.back() == '/'))
			filename = curDirectory + "\\" + filename;
		else filename = curDirectory + filename;
	}
	filename = System::fixPath(filename);

	size_t filesize;
	auto file = openFile(filename, filesize);
	if(file)
	{
		// get directory
		curDirectory = filename;
		auto lastSlash = curDirectory.find_last_of("\\");
		if (lastSlash == std::string::npos)
			lastSlash = curDirectory.find_last_of("/");

		if(lastSlash != std::string::npos)
		{
			curDirectory = curDirectory.substr(0, lastSlash + 1);
		}
		else curDirectory = "";

		parse(file, filesize, scene, curDirectory, filename);
		return true;
	}
	System::error("cannot open file " + filename);
	return false;
}