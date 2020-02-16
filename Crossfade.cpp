#include "Crossfade.h"
#include <cmath>

bool Crossfade::isCrossfading = false;
bool Crossfade::beginning = true;
double Crossfade::x1 = 0; 
double Crossfade::x2 = 0;
double Crossfade::time = -1; 

Crossfade::Crossfade() 
{
}

Crossfade::~Crossfade()
{
}

double Crossfade::getCrossfadeValue() {
	return createCrossfade();
}

bool Crossfade::getIsCrossfading() {
	return isCrossfading;
}

void Crossfade::setIsCrossfading(bool isCrossfading_1) {
	isCrossfading = isCrossfading_1;
}

void Crossfade::startCrossfading(double x_1, double x_2) {
	time = -1; 
	x1 = x_1;
	x2 = x_2;
}

bool Crossfade::atBeginning() {
	if (time == -1) {
		return true;
	}
	return false; 
}

// Creates equal power crossfade 
// x1 is where going to, x2 is where coming from 
double Crossfade::createCrossfade() {
	double volumeY1 = sqrt(.5 * (1 + time)); 
	double volumeY2 = sqrt(.5 * (1 - time));
	double y1 = x1 * volumeY1;
	double y2 = x2 * volumeY2;
	time += .1; 
	if (time > 1) { 
		setIsCrossfading(false); 
		time = -1; 
	}
	return y1 + y2; 
}

