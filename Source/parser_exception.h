#pragma once
#include <exception>
#include <string>

class ParserException : public std::exception
{
public:
	ParserException(const char* at, std::string msg)
		:
		m_at(at), m_msg(msg)
	{}
	virtual ~ParserException() throw() override
	{}

	virtual char const* what() const override
	{
		return m_msg.c_str();
	}
	const char* where() const
	{
		return m_at;
	}
private:
	const char* m_at;
	std::string m_msg;
};

#define PARSER_EXCEPT_CLASS(name, message, ...) class name : public ParserException {\
												public: name(const char* at, __VA_ARGS__)\
												: ParserException(at, message){}}

#define PBRT_EXEPT_CLASS(name, message, ...) class name : public std::exception {\
												public: name(__VA_ARGS__)\
												: exception((message).c_str()){}}

PARSER_EXCEPT_CLASS(InvalidToken, 
	"invalid token, found: " + std::string(&found, 1) + " expected: " + expected, 
	char found, std::string expected);

PARSER_EXCEPT_CLASS(InvalidArgCount,
	"invalid number of argument, found: " + std::to_string(found) + " expected: " + std::to_string(expected),
	size_t found, size_t expected);

PARSER_EXCEPT_CLASS(InvalidArgument,
	"invalid argument found: " + arg,
	std::string arg);

PARSER_EXCEPT_CLASS(ConversionError,
	"cannot convert " + number + " to " + type,
	std::string number, std::string type);

PBRT_EXEPT_CLASS(PbrtMissingParameter, "missing required parameter: " + param, std::string param);
PBRT_EXEPT_CLASS(PbrtArgMismatch, "argument count mismatch for " + name, std::string name);