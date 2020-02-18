#pragma once
class Crossfade
{
public:

	static bool getIsCrossfading();
	static void setIsCrossfading(bool isCrossfading);
	static double createCrossfade(double y1, double y2, double time);

private:
	Crossfade();
	~Crossfade();

	static bool isCrossfading;
};

