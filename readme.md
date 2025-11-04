This is my implementation of Connect 4. I am on WSL so I modified the main_macos.cpp a little bit in order to run my code properly.

The main struggle with this assignment was to create a wellrounded heuristic that handles basic cases and is not computationally expensive.

My first step was to make sure all the mechanics of Connect4 were working, meaning all of the moves had to be valid, the win conditions set, and make sure that process is efficient.

After that, I implemented the negamax algorithm with alpha-beta pruning. I made sure to include multiple optimizations as well, such as preordering the columns and using 
a Zobrist cache (which I read a lot into, they're super cool and increase the perf a lot). As for the performance of my AI, it is able to reach a depth of 15 and make moves 
in less than 5 seconds, however, there were a few instances where my AI was lacking. One of the main problems that I could not solve for some reason was double threats, were
the player would put in a delayed fork, and the AI would not be able to figure out what to do against that. I tried my best to solve this problem, but I could not find a good
solution.
