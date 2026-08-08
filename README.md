# cPractise
Just a way to learn c. Following the handmade hero series and implementing the same ideas with SDL library. Below I will document mistakes that I did and the solutions, best practises that I learned and notes related to complex c concepts that I can revisit any time.

=====================================================================================

When I wanted to create the audio buffer I did a mistake. I created a global variable *static int current_sine_sample = 0;* to keep track of the sound wave cycle. Creating that one in the .dylib (hello.cpp) it actually results to a bug. The problem that creates is with hot reloading. Each time that you change the code, you will recompile the .dylib and that will result to the reset of the variable. A better approach would be to create that global variable in the .exe and store it to it's own memory address. In that way the software can continue where is left.

=====================================================================================

typedef gives a nickname to the struct, so that when you use it you don't have to write the keyword struct.

=====================================================================================

Creating sound buffers is not that easy. I did spend 6 hour study to understand a small part of the concept. Let me explain you what I got till know. 
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

=====================================================================================

important c concept: when you cast to a buffer a specific type, for example 16bit integer, it goes and does the below 
buffer[0] -> bytes 0x1000-0x1001
buffer[1] -> bytes 0x1002-0x1003
notice that the memory it self does not change, but the compiler's interpatation of memory changes.
if we casted the buffer to a float, then the below would happened
buffer[0] -> bytes 0x1000-0x1003
buffer[1] -> bytes 0x1004-0x1007

=====================================================================================

seperation of platform layer and game layer. for example for keyboard inputs, the platform layer should inform the game layer what action should be done. 

=====================================================================================

in the day 18 we tried to synch the monitor refresh rate with the game frame rate. to be clear we didn't synch them but we made the frame rate to be steady, a step towards synch. if some frames take longer than the monitor refresh interval, we can miss the opportunity to present a new frame during that refresh, causing inconsistent motion.  so we need a variable at the start of the frame and then at the end (after we update, render) to know how many miliseconds elapsed. if that time was less than the target (the target for now is set from us), we calculate the remaining time and sleep only for that remaining duration. we set the CPU to "sleep mode", to rest or do any other tasks. for now we don't handle the case where the seconds elapsed for the frame to end is bigger than the target.

=====================================================================================

watching videos in youtube on game development with c/c++, there were cases where they used extern c in the function declaration file. what are the use cases? when to use it? after little bit of searching found that functions c are compiled differently than c++. the reason is "function overloading". what that means? in c you must have different namings for each function, duplications are not allowed, because function overloading is not supported. so functions in c when are compiled, in binary they keep the name of the function, for example if the function was void* add(something) then the compiled binary will look something like _add. C++ supports function overloading, you can create functions with the same name, you just need to have different parameters. In C++ a function void* add(something) will be compiled into _Z3iaaii. And here the extern c comes where it disables the function name mangling.

=====================================================================================

how calculating pitch works:

Think of it like a spreadsheet

Suppose each row has 10 cells.

Row 0: cells  0-9
Row 1: cells 10-19
Row 2: cells 20-29
Row 3: cells 30-39

To find the first cell of row r:

row_start = r * cells_per_row

For row 3:

3 * 10 = 30

You don't do:

3 + 10 = 13

because that lands somewhere in the middle of row 1.

The same idea applies to bitmap memory.

=====================================================================================
