#pragma once
#include "Drawable.h"

struct PhysicallyDrawable : public Drawable
{
	virtual const PBRParams getPBRParams() = 0;
	virtual void setPBRParams(PBRParams params) = 0;
};