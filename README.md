# cPractise
Just a way to learn c. Following the handmade hero series and implementing the same ideas with SDL library. Below I will document mistakes that I did and the solutions, best practises that I learned and notes related to complex c concepts that I can revisit any time.

- When I wanted to create the audio buffer I did a mistake. I created a global variable *static int current_sine_sample = 0;* to keep track of the sound wave cycle. Creating that one in the .dylib (hello.cpp) it actually results to a bug. The problem that creates is with hot reloading. Each time that you change the code, you will recompile the .dylib and that will result to the reset of the variable. A better approach would be to create that global variable in the .exe and store it to it's own memory address. In that way the software can continue where is left.

- typedef gives a nickname to the struct, so that when you use it you don't have to write the keyword struct.