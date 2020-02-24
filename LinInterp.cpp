#include "LinInterp.h"

LinInterp::LinInterp() {}

LinInterp::~LinInterp() {}

float LinInterp::dLinTerp(float x1, float x2, float y1, float y2, float x)
{
	if (x2 - x1 == 0) // avoid divide by 0 
	{ 
		return 0;
	}
	return (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1); 
}
