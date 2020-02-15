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
  
  double prev_out = 0; 
  double prev_out2 = 0;
  double prev_out3 = 0;
  double* mpBuffer = NULL;
  int mReadIndex = 0;
  int mWriteIndex = 0;
  int mBufferSize = 0;
    };

#endif
