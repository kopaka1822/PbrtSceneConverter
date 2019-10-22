#pragma once
#include <string>

class DialogOpenFile
{
public:

	// "png,jpg;pdf"
	DialogOpenFile(std::string pattern);
	~DialogOpenFile();
	void Show();
	bool IsSuccess() const;
	std::string GetName() const;
private:
	const std::string patt;
	std::string result;
	bool bSucces = false;
};