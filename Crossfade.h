#pragma once
class Crossfade
{
public:

	static bool getIsCrossfading();
	static double getCrossfadeValue();
	static void startCrossfading(double x1, double x2);
	static void setIsCrossfading(bool isCrossfading);
	static bool atBeginning();

private:
	Crossfade();
	~Crossfade();

	static double createCrossfade();
	static bool isCrossfading;
	static bool beginning;
	static bool currentValue;
	static double time; 
	static double x1, x2; 
};

