#include "DialogOpenFile.h"
#include "filedialog/include/nfd.h"

DialogOpenFile::DialogOpenFile(std::string pattern)
	:
	patt(pattern)
{}
DialogOpenFile::~DialogOpenFile()
{}

void DialogOpenFile::Show()
{
	nfdchar_t* outPath = nullptr;
	nfdresult_t nres = NFD_OpenDialog((nfdchar_t*)patt.c_str(), nullptr, &outPath);
	if (nres == NFD_OKAY)
	{
		bSucces = true;
		result = (char*)outPath;
		free(outPath);
		outPath = nullptr;
	}
}
bool DialogOpenFile::IsSuccess() const
{
	return bSucces;
}
std::string DialogOpenFile::GetName() const
{
	return result;
}