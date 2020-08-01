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
	*out1 = rdGenerator.generateDelay(*input1);
    //because we are working in mono we'll just copy the left output to the right output.
    *out2 = *out1;
  }
}



void DelayPlugin::Reset()
{
  TRACE;
  IMutexLock lock(this);
  
  rdGenerator.setSampleRate(GetSampleRate());

}

void DelayPlugin::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

	switch (paramIdx)
	{
	case kChange:
		rdGenerator.setChange(GetParam(kChange)->Value());
		break; 
	case kRandom:
		rdGenerator.setRandom(GetParam(kRandom)->Value());
		break;
	case kDelayMS:
		rdGenerator.setDelayMS(GetParam(kDelayMS)->Value()); 
		break; 
	case kFeedbackPC:
		rdGenerator.setFeedbackPC(GetParam(kFeedbackPC)->Value());
		break;
	case kWetPC:
		rdGenerator.setWetnessPC(GetParam(kWetPC)->Value());
		break; 
	default:
		break;
	}
}


