#include "Drawable.h"

Drawable::DrawError::DrawError(int line, const char* file) :
	BaseException(line, file)
{
}

const char* Drawable::DrawError::GetType() const noexcept
{
	return "Draw Error";
}