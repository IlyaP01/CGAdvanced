#pragma once
#include "WinDef.h"
#include <exception>
#include <string>

class BaseException : public std::exception
{
public:
	BaseException( int line,const char* file ) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	int line;
	std::string file;
protected:
	mutable std::string whatBuffer;
};


template <typename T>
void ThrowIfFailed(HRESULT hr, int line, const char* file)
{
	if (!SUCCEEDED(hr))
		throw T(line, file);
}
#define THROW_IF_FAILED(except, hr) ThrowIfFailed<except>(hr, __LINE__, __FILE__)
