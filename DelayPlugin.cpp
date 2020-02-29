#include "DelayPlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "IKnobMultiControlText.h"
#include "Crossfade.h"
#include "LinInterp.h"

const int kNumPrograms = 1;

enum EParams
{
  kDelayMS,
  kFeedbackPC,
  kWetPC,
  kChange,
  kRandom,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = 32,
  kGainY = 32,
  kKnobFrames = 60
};

void DelayPlugin::CreatePresets() {
	MakePreset("clean", 0.0, 0.0, 50.0, 1, 0.0);
	MakePreset("did i stutter??", 15.0, 85.0, 50.0, 12, 83.0);
	MakePreset("funk soul brother", 200.0, 85.0, 50.0, 1, 100.0);
	MakePreset("oh no", 95.0, 82, 100.0, 19, 100.0);
	MakePreset("gallop", 700.00, 47, 50.0, 100, 5.0);
}

DelayPlugin::DelayPlugin(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kDelayMS)->InitDouble("Delay", 10., 0., 2000., 0.01, "Milliseconds");
  GetParam(kFeedbackPC)->InitDouble("Feedback", 50., 0., 100.0, 0.01, "%");
  GetParam(kChange)->InitInt("Change Randomness", 1, 1, 100, "%");
  GetParam(kRandom)->InitDouble("Randomness", 0., 0., 100.0, 0.01, "%");
  GetParam(kWetPC)->InitDouble("Wet/Dry", 50., 0., 100.0, 0.01, "%");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

  IText mKnobText(10, &COLOR_BLACK, "Arial", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityDefault);
  IText controlText(14, &COLOR_BLACK, "Arial", IText::kStyleItalic, IText::kAlignNear, 0, IText::kQualityDefault);

  IRECT tmpRect(22, 189, 200, 30);
  pGraphics->AttachControl(new ITextControl(this, tmpRect, &controlText, "Delay"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(20, 209, (&knob)->W + 20, ((&knob)->H / (&knob)->N) + 229), kDelayMS, &knob, &mKnobText));

  IRECT tmpRect2(72, 189, 200, 30);
  pGraphics->AttachControl(new ITextControl(this, tmpRect2, &controlText, "Feedback"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(80, 209, (&knob)->W + 80, ((&knob)->H / (&knob)->N) + 229), kFeedbackPC, & knob, &mKnobText));

  IRECT tmpRect3(195, 189, 200, 30);
  pGraphics->AttachControl(new ITextControl(this, tmpRect3, &controlText, "Change"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(200, 209, (&knob)->W + 200, ((&knob)->H / (&knob)->N) + 229), kChange, &knob, &mKnobText));

  IRECT tmpRect4(135, 189, 200, 30);
  pGraphics->AttachControl(new ITextControl(this, tmpRect4, &controlText, "Random"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(140, 209, (&knob)->W + 140, ((&knob)->H / (&knob)->N) + 229), kRandom, &knob, &mKnobText));

  IRECT tmpRect5(255, 189, 200, 30);
  pGraphics->AttachControl(new ITextControl(this, tmpRect5, &controlText, "Dry/Wet"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(260, 209, (&knob)->W + 260, ((&knob)->H / (&knob)->N) + 229), kWetPC, &knob, &mKnobText));

  AttachGraphics(pGraphics);

  CreatePresets();
}

DelayPlugin::~DelayPlugin()
{
  if(mpBuffer)
  {
    delete [] mpBuffer;
  }
}

void DelayPlugin::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  double* input1 = inputs[0];
  double* input2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++input1, ++input2, ++out1, ++out2) // for loop to cycle through frame.
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
		sample = *input1; 
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
      sample = *input1;
	}
	else 
	{
		sample = fInterp;
	}

    //now we write to out delay buffer
    mpBuffer[mWriteIndex] = *input1 + mFeedback * sample;
    
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

	//.. and then perform the calculation for the output. Notice how the *in is factored by 1 - mWet (which gives the dry level, since wet + dry = 1)
	*out1 = (mWet * sample + (1 - mWet) * *input1);
	UpdateIndexes(); 

    //because we are working in mono we'll just copy the left output to the right output.
    *out2 = *out1;
  }
}

void DelayPlugin::newRandomIndex() {
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
		randomIndex = (lowRange + (std::rand() % (highRange - lowRange))) % mBufferSize; // pick sample within user specified range of delay 
	}

	if (randomIndex < 0) randomIndex + mBufferSize;
	randCount++;
	Crossfade::setIsCrossfading(true); // jumping to new sample, need to crossfade with previous
	crossfadeTime = -.9; // start crossfading with most of previous sample, and a little of new sample 
}

void DelayPlugin::UpdateIndexes() //then we increment the write index, wrapping if it goes out of bounds.
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

void DelayPlugin::Reset()
{
  TRACE;
  IMutexLock lock(this);
  
  mBufferSize = 2*GetSampleRate();
  if(mpBuffer)
  {
    delete [] mpBuffer;
  }
  
  mpBuffer = new double[mBufferSize];
  
  resetDelay();
  
  cookVars();
}

void DelayPlugin::resetDelay()
{
  if(mpBuffer)
  {
    memset(mpBuffer, 0, mBufferSize*sizeof(double));
  }
  
  mWriteIndex = 0;
  mReadIndex = 0;
}

void DelayPlugin::cookVars()
{
  mDelaySam = GetParam(kDelayMS)->Value() * GetSampleRate() / 1000.0;
  mFeedback = GetParam(kFeedbackPC)->Value() / 100.0;
  mWet = GetParam(kWetPC)->Value() / 100.0;
  
  mReadIndex = mWriteIndex - (int)mDelaySam;
  if(mReadIndex < 0)
  {
    mReadIndex += mBufferSize;
  }
}

void DelayPlugin::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

	switch (paramIdx)
	{
	case kChange:
		mChange = GetParam(kChange)->Value(); 
		break; 
	case kRandom:
		mRandom = GetParam(kRandom)->Value() / 100;
	default:
		break;
	}
  cookVars();
}


