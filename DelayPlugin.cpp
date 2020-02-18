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

  kGainX = 125,
  kGainY = 125,
  kKnobFrames = 60
};

void DelayPlugin::CreatePresets() {
	MakePreset("clean", 0.0, 0.0, 50.0, 1, 0.0);
	MakePreset("did i stutter??", 15.0, 85.0, 50.0, 12, 83.0);
	MakePreset("funk soul brother", 200.0, 85.0, 50.0, 1, 100.0);
	MakePreset("oh no", 150.0, 100.0, 100.0, 8, 100.0);
}


DelayPlugin::DelayPlugin(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), useRand(true), randCount(0), randomIndex(0)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kDelayMS)->InitDouble("Delay", 10., 0., 200., 0.01, "Milliseconds");
  GetParam(kFeedbackPC)->InitDouble("Feedback", 50., 0., 100.0, 0.01, "%");
  GetParam(kChange)->InitInt("Change Randomness", 1, 1, 100, "%");
  GetParam(kRandom)->InitDouble("Randomness", 0., 0., 100.0, 0.01, "%");
  GetParam(kWetPC)->InitDouble("Wet/Dry", 50., 0., 100.0, 0.01, "%");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

  pGraphics->AttachPanelBackground(&COLOR_GRAY);
  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

  IRECT tmpRect(20, 180, 200, 30);
  IText textProps(14, &COLOR_BLACK, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new ITextControl(this, tmpRect, &textProps, "Delay"));

  IText *mText = new IText(11, &COLOR_BLACK, "Arial", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(20, 200, (&knob)->W + 20, ((&knob)->H / (&knob)->N) + 220), kDelayMS, &knob, mText));

  IRECT tmpRect2(80, 180, 200, 30);
  IText textProps2(14, &COLOR_BLACK, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new ITextControl(this, tmpRect2, &textProps2, "Feedback"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(80, 200, (&knob)->W + 80, ((&knob)->H / (&knob)->N) + 220), kFeedbackPC, & knob, mText));

  IRECT tmpRect3(200, 180, 200, 30);
  IText textProps3(14, &COLOR_BLACK, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new ITextControl(this, tmpRect3, &textProps3, "Change"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(200, 200, (&knob)->W + 200, ((&knob)->H / (&knob)->N) + 220), kChange, &knob, mText));

  IRECT tmpRect4(140, 180, 200, 30);
  IText textProps4(14, &COLOR_BLACK, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new ITextControl(this, tmpRect4, &textProps4, "Random"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(140, 200, (&knob)->W + 140, ((&knob)->H / (&knob)->N) + 220), kRandom, &knob, mText));

  IRECT tmpRect5(260, 180, 200, 30);
  IText textProps5(14, &COLOR_BLACK, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
  pGraphics->AttachControl(new ITextControl(this, tmpRect5, &textProps5, "Dry/Wet"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, *new IRECT(260, 200, (&knob)->W + 260, ((&knob)->H / (&knob)->N) + 220), kWetPC, &knob, mText));

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

  double* xn1 = inputs[0];
  double* xn2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++xn1, ++xn2, ++out1, ++out2) // for loop to cycle through frame.
  { 
	  //first we read our delayed output
	  double yn;

	  if (randCount == 0) { // if randCount 0, need to choose a new starting location for samples
		  oldIndex = randomIndex; // save randomIndex current location before changing, so we know where to crossfade from 
		  int range = mReadIndex * mRandom; // cannot change mReadIndex, as that is controlled by the user, so will use randomIndex 
		  int lowRange = mReadIndex - range; 
		  int highRange = mReadIndex + range; 
		  if (highRange == lowRange) (++highRange); // to prevent division by 0 

		  randomIndex = (lowRange + (std::rand() % (highRange - lowRange))) % mBufferSize; // pick sample within user specified range of delay 
		  if (randomIndex < 0) randomIndex + mBufferSize;
		  randCount++;
		  Crossfade::setIsCrossfading(true); // jumping to new sample, need to crossfade with previous
		  crossfadeTime = -.9; // start crossfading with most of previous sample, and a little of new sample 
	  }
	  else { // else continue sweeping through buffer 
		  randomIndex = ++randomIndex;
		  oldIndex == ++oldIndex;
		  if (randomIndex > mBufferSize) randomIndex = 0; 
		  if (oldIndex > mBufferSize) oldIndex = 0; 
		  randCount++;
	  }
	  if (randCount > mBufferSize / mChange || randCount >= mBufferSize) randCount = 0; // make sure randCount stays within bounds


	yn = mpBuffer[randomIndex];
	// if delay < 1 sample, interpolate between input x(n) and x(n-1)
	if (randomIndex == mWriteIndex && mDelaySam < 1.00)
	{
		// interpolate current input with input one sample behind
		yn = *xn1; 
	}

	float yn_1;
	int mRandomIndex_1 = 0;
	// read location one behind yn at y(n-1)
	mRandomIndex_1 = randomIndex - 1;

	if (mRandomIndex_1 < 0) mRandomIndex_1 = mBufferSize - 1; // if wrapping around buffer 
	yn_1 = mpBuffer[mRandomIndex_1];

		//// interpolate: 0, 1 for DSP range, yn to yn-1 for user defined range. 
	float fFracDelay = mDelaySam - (int)mDelaySam; // by casting to int, find fraction between delay samples

	float fInterp = LinInterp::dLinTerp(0, 1, yn, yn_1, fFracDelay); 

    //if the delay is 0 samples we just feed it the input
    if (mDelaySam == 0)
    {
      yn = *xn1;
	}
	else 
	{
		yn = fInterp;
	}

    //now we write to out delay buffer
    mpBuffer[mWriteIndex] = *xn1 + mFeedback * yn;
    
	if (Crossfade::getIsCrossfading()) { // if currently performing a crossfade between 2 values, continue doing crossfade 
		double oldSample = mpBuffer[oldIndex]; // don't think we need to perform linear interpolation on this value, as it already contains linearly interpolated old sample 

		yn = Crossfade::createCrossfade(yn, oldSample, crossfadeTime);
		crossfadeTime += .1;
		if (crossfadeTime >= 1.0) { // once reach 1, current sample fully playing, and have finished crossfade 
			Crossfade::setIsCrossfading(false); 
			crossfadeTime = -1.0; 
		}
	}

	//.. and then perform the calculation for the output. Notice how the *in is factored by 1 - mWet (which gives the dry level, since wet + dry = 1)
	*out1 = (mWet * yn + (1 - mWet) * *xn1);


	//then we increment the write index, wrapping if it goes out of bounds.
    ++mWriteIndex;
    if(mWriteIndex >= mBufferSize)
    {
      mWriteIndex = 0;
    }
    
    //same with the read index
    ++mReadIndex;
    if(mReadIndex >= mBufferSize)
    {
      mReadIndex = 0;
    }
    

    //because we are working in mono we'll just copy the left output to the right output.
    *out2 = *out1;
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


