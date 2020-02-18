#include "Crossfade.h"
#include <cmath>

bool Crossfade::isCrossfading = false;

Crossfade::Crossfade() 
{
}

Crossfade::~Crossfade()
{
}

bool Crossfade::getIsCrossfading() {
	return isCrossfading;
}

void Crossfade::setIsCrossfading(bool isCrossfading_1) {
	isCrossfading = isCrossfading_1;
}

// Creates equal power crossfade 
// x1 is where going to, x2 is where coming from 
double Crossfade::createCrossfade(double x1, double x2, double time) {
	double volumeY1 = sqrt(.5 * (1 + time)); 
	double volumeY2 = sqrt(.5 * (1 - time));
	double y1 = x1 * volumeY1;
	double y2 = x2 * volumeY2;
	return y1 + y2; 
}

