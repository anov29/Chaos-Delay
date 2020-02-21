# Chaos Delay
A randomized delay made by Andrew Novitskiy 

Delay knob up goes up to 2000ms. 

Random knob sets the spread of samples that Chaos Delay chooses from the feedback buffer. At 100, every delayed sample can randomly be chosen. 

Change knob sets how often Chaos Delay chooses a new random sample. When Random is 0, Change has no effect.  

Built on top of the WDL iPlug framework

A big thanks to Will Prikle for his book on Designing Audio Effects in C++, and Luke Zeitlin for his work on the WDL framework and his online delay tutorial. 

//TODO: Add tempo synced delay

//TODO: Improve crossfade function

//TODO: Make delay more than 200ms??
