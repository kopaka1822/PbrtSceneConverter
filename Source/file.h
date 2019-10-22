#pragma once
#include <memory>
#include <string>
#include <iostream>

inline std::unique_ptr<char[]> openFile(const std::string& filename, size_t& filesize)
{
	std::unique_ptr<char[]> file;

	FILE* pFile = fopen(filename.c_str(), "rb");
	if (!pFile) return file;

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	filesize = ftell(pFile);
	rewind(pFile);

	static const size_t puffer = 100;
	file = std::unique_ptr<char[]>(new char[filesize + puffer]);

	auto count = fread(file.get(), 1, filesize, pFile);
	fclose(pFile);

	// safety
	memset(file.get() + filesize, 0, puffer);
	
	if (count != filesize)
		System::warning("could not read all bytes from " + filename);

	return file;
}

inline void saveFile(const std::string& filename, const std::string& txt)
{
	FILE* pFile = fopen(filename.c_str(), "w");
	if(!pFile)
	{
		System::error("cannot save " + filename);
		return;
	}

	fprintf(pFile, txt.c_str());

	fclose(pFile);
}