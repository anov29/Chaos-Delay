#include "RandomDelayGenerator.h"
#include "vcruntime_string.h"
#include "Crossfade.h"
#include "LinInterp.h"
#include <stdlib.h>

RandomDelayGenerator::RandomDelayGenerator()
{
}


RandomDelayGenerator::~RandomDelayGenerator()
{
	if (mpBuffer)
	{
		delete[] mpBuffer;
	}
}

void RandomDelayGenerator::setDelayMS(double delayMS)
{
	this->mDelaySam = delayMS * mSampleRate / 1000.0; 
	mReadIndex = mWriteIndex - (int)mDelaySam;
	if (mReadIndex < 0)
	{
		mReadIndex += mBufferSize;
	}
}

void RandomDelayGenerator::setChange(double change)
{
	this->mChange = change;
}

void RandomDelayGenerator::setRandom(double random)
{
	this->mRandom = random;
}

void RandomDelayGenerator::setFeedbackPC(double feedbackPC)
{
	this->mFeedback = feedbackPC / 100.0; 
}

void RandomDelayGenerator::setWetnessPC(double wetPC)
{
	this->mWet = wetPC / 100.0; 
}

void RandomDelayGenerator::setSampleRate(double sampleRate)
{
	this->mSampleRate = sampleRate;
	mBufferSize = 2 * mSampleRate;
	if (mpBuffer)
	{
		delete[] mpBuffer;
	}

	mpBuffer = new double[mBufferSize];
	if (mpBuffer)
	{
		memset(mpBuffer, 0, mBufferSize * sizeof(double));
	}

	mWriteIndex = 0;
	mReadIndex = 0;
}

double RandomDelayGenerator::generateDelay(double input)
{
	//first we read our delayed output
	double sample;

	if (randCount == 0) // if randCount 0, need to choose a new starting location for samples
	{
		newRandomIndex();
	}
	else // else continue sweeping through buffer 
	{
		randomIndex = ++randomIndex;
		oldIndex = ++oldIndex;
		if (randomIndex > mBufferSize) randomIndex = 0;
		if (oldIndex > mBufferSize) oldIndex = 0;
		randCount++;
	}
	if ((randCount > mBufferSize / mChange || randCount >= mBufferSize) && randCount != 1) randCount = 0; // make sure randCount stays within bounds. If randCount 1, ignore value 

	sample = mpBuffer[randomIndex];
	// if delay < 1 sample, interpolate between input x(n) and x(n-1)
	if (randomIndex == mWriteIndex && mDelaySam < 1.00)
	{
		// interpolate current input with input one sample behind
		sample = input;
	}

	float yn_1;
	int mRandomIndex_1 = 0;
	// read location one behind yn at y(n-1) 
	mRandomIndex_1 = randomIndex - 1;

	if (mRandomIndex_1 < 0) mRandomIndex_1 = mBufferSize - 1; // if wrapping around buffer 
	yn_1 = mpBuffer[mRandomIndex_1];

	//// interpolate: 0, 1 for DSP range, yn to yn-1 for user defined range. 
	float fFracDelay = mDelaySam - (int)mDelaySam; // by casting to int, find fraction between delay samples
	float fInterp = LinInterp::dLinTerp(0, 1, sample, yn_1, fFracDelay);

	//if the delay is 0 samples we just feed it the input
	if (mDelaySam == 0)
	{
		sample = input;
	}
	else
	{
		sample = fInterp;
	}

	//now we write to out delay buffer
	mpBuffer[mWriteIndex] = input + mFeedback * sample;

	if (Crossfade::getIsCrossfading()) // if currently performing a crossfade between 2 values, continue doing crossfade 
	{
		double oldSample = mpBuffer[oldIndex]; // don't think we need to perform linear interpolation on this value, as it already contains linearly interpolated old sample 

		sample = Crossfade::createCrossfade(sample, oldSample, crossfadeTime);
		crossfadeTime += .01;
		if (crossfadeTime >= 1.0) // once reach 1, current sample fully playing, and have finished crossfade
		{
			Crossfade::setIsCrossfading(false);
			crossfadeTime = -1.0;
		}
	}
	UpdateIndexes();
	//.. and then perform the calculation for the output. Notice how the *in is factored by 1 - mWet (which gives the dry level, since wet + dry = 1)
	return (mWet * sample + (1 - mWet) * input);

}

void RandomDelayGenerator::newRandomIndex()
{
	oldIndex = randomIndex; // save randomIndex current location before changing, so we know where to crossfade from 
	int range = mBufferSize * mRandom; // range will fully cover buffer at random = 1
	int lowRange = mReadIndex - range;
	if (lowRange < 0) lowRange = 0;
	int highRange = mReadIndex + range;
	if (highRange >= mBufferSize) highRange = mBufferSize - 1;
	if (highRange == lowRange) // if same, random at 1, no difference between random and read index
	{
		randomIndex = mReadIndex; // cannot change mReadIndex, as that is controlled by the user, so will use randomIndex 
	}
	else
	{
		randomIndex = (lowRange + (rand() % (highRange - lowRange))) % mBufferSize; // pick sample within user specified range of delay 
	}

	if (randomIndex < 0) randomIndex + mBufferSize;
	randCount++;
	Crossfade::setIsCrossfading(true); // jumping to new sample, need to crossfade with previous
	crossfadeTime = -.9; // start crossfading with most of previous sample, and a little of new sample 

}

void RandomDelayGenerator::UpdateIndexes()
{
	++mWriteIndex;
	if (mWriteIndex >= mBufferSize)
	{
		mWriteIndex = 0;
	}

	//same with the read index
	++mReadIndex;
	if (mReadIndex >= mBufferSize)
	{
		mReadIndex = 0;
	}
}
