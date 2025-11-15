Our project is a reaction time game that uses one board. It uses the two side switches on the board (the buttons) and then a 4-digit LCD for score display, enabling two players to compete by pressing their designated buttons as quickly as possible. We will also add a fake, where if the red LED flashes, and a player clicks it, they will be penalized and lose a point. However, if the green LED flashes, the game records which player reacts first, awards a point for each win (the right button will get the digit on the rightmost side, and the same for left), and continues until one player gets 3 points. This will use advanced scheduling techniques by managing RT processes for accurately measuring reaction times and NRT processes for displaying updates on the LCD. 

## Peripherals: 

- LEDS: signal the start of each round and occasionally create fake outs

- 2 switches on the board: capture user input, interfaced with the microcontroller 

- 4-Digit LCD display: used to show current score

- Timers and Interrupts: The project uses two timers. One timer helps switch between tasks in the scheduler, and the other keeps track of time for the reaction game. The second timer is set to go off every 1 millisecond, and each time it does, it updates a global time variable. This lets the program measure how fast each player reacts with accurate timing.


## Software & Processes: 

- Real-Time (RT) Processes: Use the 1ms timer to track when each player presses their button. When a round begins, the program randomly chooses whether to flash the LED as a real signal or as a fake-out. Button presses are timestamped, and if a player reacts:

  * Too early during a fake-out, they lose 1 point (but not below 0).

  * Correctly during a real signal, the first to react gains a point.

- Non-Real-Time (NRT) Processes: These are background tasks like updating the LCD with scores or running basic game logic. They don’t need exact timing, so they take turns running using a round-robin method that shares time fairly between them.

- How They Work Together: If a real-time task is ready to run, it will pause whatever non-real-time task is running. If no tasks are ready, the system will wait until something happens, like a button press. This setup makes sure the game runs smoothly while keeping accurate track of who reacts first.
