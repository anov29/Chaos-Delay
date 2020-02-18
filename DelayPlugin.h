#ifndef __DELAYPLUGIN__
#define __DELAYPLUGIN__

#include "IPlug_include_in_plug_hdr.h"

class DelayPlugin : public IPlug
{
public:
  DelayPlugin(IPlugInstanceInfo instanceInfo);
  ~DelayPlugin();

  float DelayPlugin::dLinTerp(float x1, float x2, float y1, float y2, float x);
  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void cookVars();
  void resetDelay();
  
  private:
	  bool useRand; 
	  int randCount;
	  int randomIndex; 

  double mDelaySam = 0.;
  double mFeedback = 0.;
  double mWet = 0.;
  int mChange = 1; // how often to change mRandom 
  double mRandom = 0; // percent from 0 to 1, where 1 = 100% of buffer avaliable 
  
  double crossfadeTime = -1.0;  // range from [-1, 1], where -1 previous sample, and 1 current sample
  double prev_out = 0; 
  double prev_out2 = 0;
  double prev_out3 = 0;
  double* mpBuffer = NULL;
  int mReadIndex = 0;
  int mWriteIndex = 0;
  int oldIndex = 0; // where read index used to be before crossfade 
  double oldDelaySam = 0; // Old delay sample to use for interpolation
  int mBufferSize = 0;
    };

#endif
