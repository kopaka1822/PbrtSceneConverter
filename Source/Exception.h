#pragma once
#include <exception>
#include <string>

class Exception : public std::exception
{
	
public:
	Exception(std::string msg)
		:
		m_msg(msg)
	{}
	virtual ~Exception() throw() override
	{}

	virtual char const* what() const override
	{
		return m_msg.c_str();
	}
private:
	std::string m_msg;
};
