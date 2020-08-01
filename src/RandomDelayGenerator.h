#pragma once
class RandomDelayGenerator
{
public:
	RandomDelayGenerator();
	~RandomDelayGenerator();

	void setDelayMS(double delayMS);
	void setChange(double change);
	void setRandom(double random);
	void setFeedbackPC(double feedbackPC);
	void setWetnessPC(double wetPC);
	void setSampleRate(double sampleRate);

	double generateDelay(double input);

private:
	void newRandomIndex();
	void UpdateIndexes();

	double mRandom = 0; // percent from 0 to 1, where 1 = 100% of buffer avaliable 
	double mDelaySam = 0.;
	double mFeedback = 0.;
	double mWet = 0.;
	double crossfadeTime = -1.0;  // range from [-1, 1], where -1 previous sample, and 1 current sample
	double prev_out = 0;
	double prev_out2 = 0;
	double prev_out3 = 0;

	double* mpBuffer = nullptr;

	double mSampleRate = 44100; 
	int randCount = 0;
	int randomIndex = 0;
	int mChange = 1; // how often to change mRandom 
	int mReadIndex = 0;
	int mWriteIndex = 0;
	int oldIndex = 0; // where read index used to be before crossfade 
	int mBufferSize = 0;

};

