#include "DelayPlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kDelayMS,
  kFeedbackPC,
  kWetPC,
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


DelayPlugin::DelayPlugin(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label

  
  GetParam(kDelayMS)->InitDouble("Delay", 10., 0., 200., 0.01, "Milliseconds");
  GetParam(kFeedbackPC)->InitDouble("Feedback", 50., 0., 100.0, 0.01, "%");
  GetParam(kWetPC)->InitDouble("Wet/Dry", 50., 0., 100.0, 0.01, "%");
  
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

  pGraphics->AttachPanelBackground(&COLOR_GRAY);
  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

  pGraphics->AttachControl(new IKnobMultiControl(this, 20, 200, kDelayMS, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 80, 200, kFeedbackPC, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 140, 200, kWetPC, &knob));
  
  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
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
    double yn = mpBuffer[mReadIndex];
        
	// if delay < 1 sample, interpolate between input x(n) and x(n-1)
	if (mReadIndex == mWriteIndex && mDelaySam < 1.00)
	{
		// interpolate current input with input one sample behind
		yn = *xn1; 
	}
	// read location one behind yn at y(n-1)
	int mReadIndex_1 = mReadIndex - 1;
	if (mReadIndex_1 < 0) mReadIndex_1 = mBufferSize - 1; // if wrapping around buffer 

	//// get y(n-1)
	float yn_1 = mpBuffer[mReadIndex_1];

	//// interpolate: 0, 1 for DSP range, yn to yn-1 for user defined range. 
	float fFracDelay = mDelaySam - (int)mDelaySam; // by casting to int, find fraction between delay samples

	float fInterp = DelayPlugin::dLinTerp(0, 1, yn, yn_1, fFracDelay); 

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
    
        //.. and then perform the calculation for the output. Notice how the *in is factored by 1 - mWet (which gives the dry level, since wet + dry = 1)
	*out1 = ((mWet * yn + (1 - mWet) * *xn1) * .707) + (prev_out * .707); // + (prev_out2 * .1) + (prev_out3 * .05); // + for crossfade 
	prev_out3 = prev_out2;
	prev_out2 = prev_out;
	prev_out = *out1; // rememver current out as crossfade for next out
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
  cookVars();
}

float DelayPlugin::dLinTerp(float x1, float x2, float y1, float y2, float x)
{
	if (x2 - x1 == 0) { // avoid divide by 0 
		return 0;
	}
	return (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1);
}


