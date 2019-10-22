#pragma once
#include <vector>
#include <ctype.h>
#include "parser_exception.h"
#include "PBRT/PbrtScene.h"
#include "PBRT/ParamSet.h"

enum class pbrtParamType
{
	Float,
	Integer,
	Bool,
	Point,
	Vector,
	Normal,
	String,
	Spectrum, // specturm [list] and spectrum "file" are possible
	// spectum raw types
	RGB,
	Color, // synonym for RGB
	XYZ,
	Texture,
	ERROR
};

inline const char* skipSpace(const char* c)
{
	while (*c && isspace(*c)) c++;
	return c;
}

inline const char* skipSpace(const char* c, const char* end)
{
	while (*c && c < end && isspace(*c)) c++;
	return c;
}

inline const char* skipText(const char* c)
{
	while (*c && !isspace(*c)) c++;
	return c;
}

inline const char* skipText(const char* c, const char* end)
{
	while(*c && c < end && !isspace(*c)) c++;
	return c;
}

inline std::string extractString(const char* begin, const char* end)
{
	auto s = std::string(begin, size_t(end - begin));
	return s;
}


// reads a file filled with floating point values
inline std::vector<float> readFloatingFile(std::string filename)
{
	// TODO cache spectra?
	filename = System::fixPath(System::getCurrentDirectory() + filename);
	std::vector<float> data;
	size_t size = 0;
	auto file = openFile(filename, size);
	if (!file)
		throw std::exception(("cannot open file " + filename).c_str());

	System::runtimeInfo("reading float file " + filename);
	const char* cur = file.get();
	const char* end = cur + size;

	// remove all comments
	{
		char* c = file.get();
		bool isComment = false;
		while(c < end)
		{
			if(isComment)
			{
				if (*c == '\n')
					isComment = false;
				else *c = ' ';
			}
			else if(*c == '#')
			{
				isComment = true;
				*c = ' ';
			}
			c++;
		}
	}

	cur = skipSpace(cur);

	while(*cur && cur < end)
	{
		auto send = skipText(cur);
		auto text = extractString(cur, send);
		cur = send;
		try
		{
			data.push_back(stof(text));
		}
		catch (const std::out_of_range&)
		{
			throw std::exception(("cannot convert " + text + " to float in file: " + filename + " - out of range").c_str());
		}
		catch (const std::exception&)
		{
			throw std::exception(("cannot convert " + text + " to float in file: " + filename).c_str());
		}
		cur = skipSpace(cur);
	}

	return data;
}

inline std::vector<float> getFloats(size_t num, const char*& cur, const char* end)
{
	std::vector<float> v;
	cur = skipSpace(cur);
	if (*cur == '[')
		++cur;

	while (*cur && cur < end && v.size() < num)
	{
		cur = skipSpace(cur);
		// read float
		auto send = skipText(cur);
		auto text = extractString(cur, send);
		cur = send;
		try
		{
			v.push_back(stof(text));
		}
		catch(const std::out_of_range&)
		{
			throw ConversionError(cur, text, "float - out of range");
		}
		catch(const std::exception&)
		{
			throw ConversionError(cur, text, "float");
		}
	}
	if (v.size() != num)
		throw InvalidArgCount(cur, v.size(), num);

	return v;
}

// reads next string
inline std::string getRawString(const char*& cur, const char* end)
{
	cur = skipSpace(cur);
	auto begin = cur;
	cur = skipText(cur);
	return extractString(begin, cur);
}

// gets array surrounded by two tokens
inline void getArrayBorders(const char*& cur, const char* end, const char*& abegin, const char*& aend, char sToken, char eToken)
{
	cur = skipSpace(cur);
	if (*cur != sToken)
		throw InvalidToken(cur, *cur, std::string(&sToken,1));
	// start token found
	abegin = cur++;
	// skip till next token
	while (*cur && cur < end && *cur != eToken) ++cur;

	if(*cur != eToken)
		throw InvalidToken(cur, *cur, std::string(&eToken, 1));
	aend = cur++;
}

// Camera [cursor] "perspective" ... -> perpective
inline std::string getString(const char*& cur, const char* end)
{
	const char* abegin = nullptr;
	const char* aend = nullptr;
	getArrayBorders(cur, end, abegin, aend, '"', '"');
	auto s = extractString(abegin + 1, aend);
	return s;
}

pbrtParamType getParamTypeFromString(const std::string& s);

// putting arguments between begin and end into vector -> [0 23 42] -> ["0","23","42"]
inline std::vector<std::string> getRawStringVector(const char* begin, const char* end)
{
	std::vector<std::string> args;
	while(begin < end)
	{
		begin = skipSpace(begin, end);
		if (begin >= end)break;
		// read raw string
		auto send = skipText(begin, end);
		if (send > end) send = end;
		args.push_back(extractString(begin,send));
		begin = send;
	}
	return args;
}

inline float convertRawToFloat(const std::string& s, const char* end)
{
	try
	{
		return stof(s);
	}
	catch (std::out_of_range&)
	{
		throw ConversionError(end, s, "float - out of range");
	}
	catch (const std::exception&)
	{
		throw ConversionError(end, s, "float");
	}
}
inline std::vector<float> getFloatVector(const char* begin, const char* end)
{
	std::vector<float> args;
	for(const auto& s : getRawStringVector(begin,end))
	{
		args.push_back(convertRawToFloat(s, end));
	}
	return args;
}
inline std::string convertRawToString(const std::string& s, const char* at)
{
	if (s.length() < 3)
		throw InvalidArgument(at, s);
	if (*s.begin() != '"')
		throw InvalidToken(at, *s.begin(), "\"");
	if (s.back() != '"')
		throw InvalidToken(at, s.back(), "\"");

	return std::string(s.begin() + 1, s.begin() + s.length() - 1);
}
inline std::vector<std::string> getStringVector(const char* begin, const char* end)
{
	std::vector<std::string> args;
	// wrong because -> [ "hello world" ] -> [ "hello, world" ]
	/*for(const auto& s : getRawStringVector(begin,end))
	{
		args.push_back(convertRawToString(s,end));
	}*/
	while(begin < end)
	{
		begin = skipSpace(begin, end);
		if (begin >= end)break;
		// read string
		if (*begin != '\"')
			throw InvalidToken(begin, *begin, "\"");
		auto sbegin = ++begin;

		// search string end
		while (begin < end && *begin != '\"')
			begin++;

		if (*begin != '\"')
			throw InvalidToken(begin, *begin, "\"");
		// extract string
		args.push_back(extractString(sbegin, begin++));
	}
	return args;
}
inline bool convertRawToBool(const std::string& s, const char* end)
{
	// remove parenthesis
	auto str = convertRawToString(s, end);
	if (str == "true")
		return true;
	else if (str == "false")
		return false;
	else throw InvalidArgument(end, s);
}
inline std::vector<bool> getBoolVector(const char* begin, const char* end)
{
	std::vector<bool> args;
	for(const auto& s : getRawStringVector(begin,end))
	{
		args.push_back(convertRawToBool(s, end));
	}
	return args;
}
inline int convertRawToInt(const std::string& s, const char* end)
{
	try
	{
		return stoi(s);
	}
	catch (std::out_of_range&)
	{
		throw ConversionError(end, s, "integer - out of range");
	}
	catch (const std::exception&)
	{
		throw ConversionError(end, s, "integer");
	}
}
inline std::vector<int> getIntVector(const char* begin, const char* end)
{
	std::vector<int> args;
	for (const auto& s : getRawStringVector(begin, end))
	{
		args.push_back(convertRawToInt(s, end));
	}
	return args;
}
inline std::vector<Vector> getVectorVector(const char* begin, const char* end)
{
	auto floats = getFloatVector(begin, end);
	if (floats.size() % 3 != 0)
		throw InvalidArgCount(end, floats.size(), (floats.size() / 3 + 1) * 3);
	std::vector<Vector> args;
	for(size_t i = 0; i < floats.size(); i += 3)
	{
		args.push_back(Vector(floats[i], floats[i + 1], floats[i + 2]));
	}
	return args;
}

// reads construct like: "float fov" [56]
inline void initParamSet(ParamSet& p, const char*& cur, const char* end)
{
	cur = skipSpace(cur);
	while(*cur == '"') // next is probably another argument "type name" [args]
	{
		// determine type and name
		const char* tbegin = nullptr;
		const char* tend = nullptr;
		getArrayBorders(cur, end, tbegin, tend, '"', '"');
		// split in type name
		tbegin = skipSpace(tbegin + 1); // in case we have "   type name"
		auto typeEnd = skipText(tbegin, tend);
		std::string type = extractString(tbegin, typeEnd);
		// determine name
		tbegin = skipSpace(typeEnd,tend);
		std::string name = extractString(tbegin, tend);
		// remove trailing spaces
		while (name.length() && name.back() == ' ') name.pop_back();

		// read data according to type
		// integer, float, point, vector, normal, spectrum, bool, and string
		const auto paramType = getParamTypeFromString(type);
		if (paramType == pbrtParamType::ERROR)
			throw InvalidArgument(cur, type);

		cur = skipSpace(cur);
		char nextToken = *cur;
		// because sometimes its "string name" "text" and sometimes "string name" ["text"] ...
		// and sometimes its even "float name" 60 .. (single parameter)

		if(nextToken == '[')
		{
			getArrayBorders(cur, end, tbegin, tend, '[', ']');

			++tbegin;

			// integrate data
			switch (paramType)
			{
			case pbrtParamType::Float:
				p.addFloat(name, getFloatVector(tbegin, tend));
				break;
			case pbrtParamType::Integer:
				p.addInt(name, getIntVector(tbegin, tend));
				break;
			case pbrtParamType::Point:
				p.addPoint(name, getVectorVector(tbegin, tend));
				break;
			case pbrtParamType::Vector:
				p.addVector(name, getVectorVector(tbegin, tend));
				break;
			case pbrtParamType::Normal:
				p.addNormal(name, getVectorVector(tbegin, tend));
				break;
			case pbrtParamType::Spectrum:
				p.addBlackbodySpectrum(name, getFloatVector(tbegin, tend));
				break;
			case pbrtParamType::Color:
			case pbrtParamType::RGB:
				p.addRGBSpectrum(name, getFloatVector(tbegin, tend));
				break;
			case pbrtParamType::XYZ:
				p.addXYZSpectrum(name, getFloatVector(tbegin, tend));
				break;
			case pbrtParamType::Bool:
				p.addBool(name, getBoolVector(tbegin, tend));
				break;
			case pbrtParamType::String:
				p.addString(name, getStringVector(tbegin, tend));
				break;
			case pbrtParamType::Texture:
				// assert that only one string is used!
				{
					auto tx = getStringVector(tbegin, tend);
					if(tx.size() != 1)
					{
						throw InvalidArgCount(tbegin, tx.size(), 1);
					}
					p.addTexture(name, tx.at(0));
				}
				break;
			default:
				throw InvalidArgument(tbegin, type);
			}
		}
		else
		{
			std::string s;
			// read single argument
			if(nextToken == '\"')
			{
				// read within " borders
				getArrayBorders(cur, end, tbegin, tend, '\"', '\"');
				s = extractString(tbegin, tend + 1);
			}
			else
			{
				s = getRawString(cur, end);
			}
			// String
			switch (paramType)
			{
			case pbrtParamType::String:
				p.addString(name, { convertRawToString(s, cur) });
				break;
			case pbrtParamType::Float:
				p.addFloat(name, { convertRawToFloat(s,cur) });
				break;
			case pbrtParamType::Bool:
				p.addBool(name, { convertRawToBool(s,cur) });
				break;
			case pbrtParamType::Integer:
				p.addInt(name, { convertRawToInt(s,cur) });
				break;
			case pbrtParamType::Texture:
				p.addTexture(name, { convertRawToString(s, cur) });
				break;
			case pbrtParamType::Spectrum:
				// import spectrum from file
				p.addBlackbodySpectrum(name, readFloatingFile(convertRawToString(s, cur)));
				break;
			default:
				throw InvalidToken(cur, nextToken, "[");
			}
		}
		cur = skipSpace(cur);
	}
}