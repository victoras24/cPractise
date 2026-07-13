# cPractise
Just a way to learn c. Following the handmade hero series and implementing the same ideas with SDL library. Below I will document mistakes that I did and the solutions, best practises that I learned and notes related to complex c concepts that I can revisit any time.

- When I wanted to create the audio buffer I did a mistake. I created a global variable *static int current_sine_sample = 0;* to keep track of the sound wave cycle. Creating that one in the .dylib (hello.cpp) it actually results to a bug. The problem that creates is with hot reloading. Each time that you change the code, you will recompile the .dylib and that will result to the reset of the variable. A better approach would be to create that global variable in the .exe and store it to it's own memory address. In that way the software can continue where is left.

- typedef gives a nickname to the struct, so that when you use it you don't have to write the keyword struct.

- Creating sound buffers is not that easy. I did spend 6 hour study to understand a small part of the concept. Let me explain you what I got till know. 
Sound is produced by the vibration of the speaker cone, which creates air waves that are creating sound in our ears. The problem now is how we could create a buffer that will tell to the speaker cone what movement should do. 
It's important to mention the movement that the speaker cone is doing, it starts from the center (0), then forward (1), back to center (0), then backwards (-1) and back to the center (0). If you do this very fast, it vibrates and creates the sound waves.
We will use a little bit of math to calculate that. The circle is used as a mathematical tool. Each point on the circle is called angle or phase. What we want to do is to transform those points from the circle to a vertical coordination, where in y axis is the sin value and on the x axis is the time. 
Before we do that we need to learn what the samples are and what the sample rate is. Samples is an array of values which are telling the cone how far should move. Now the frequency, which is the sample rate, are measured in Hz and it's the samples rate, how many cycles are processed in one second. 
So starting we create the variables that will store the samples and the freq.

  int16_t samples[48000];
  int sampleRate = 48000
  int feqHz = 440;

Now let's calculate how many samples each cycle processes

 float sampleCountPerCycle = sampleRate / feqHz = ~109

This variable will help us to calculate the value for each sample. So we will loop to each sample and calculate the PCM value.

   for (size_t i = 0; i < samplesLength; i++)
    {
      double angle = (i * (SDL_PI_D * 2)) / sampleCountPerCycle;
      double sinValue = sin(angle);
      samples[i] = sinValue * 10000;
    };


