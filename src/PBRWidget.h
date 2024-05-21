#pragma once
#include "PBR.h"

class PBRWidget
{
public:
	PBRWidget();
	PBRWidget(PBRParams& params);
	void AddToRender();
	PBRParams GetParams() const;
private:
	PBRParams m_params;
};