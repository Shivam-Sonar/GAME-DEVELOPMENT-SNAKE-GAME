# GAME-DEVELOPMENT-SNAKE-GAME
*COMPANY NAME*:- CODTECH IT SOLUTIONS
*NAME*:- Sonar Shivam Suryanath
*INTERN ID*:- CT04DH2913
*DOMAIN*:- C++ Programming
*DURATION* :- 4 WEEKS
*MENTOR* :- Neela Santhosh Kumar

DESCRIPTION:-
This program brings the classic Snake game to life using SFML, a multimedia library for graphics, sound, and real-time interaction. It presents a window where a snake navigates, eats food, grows, avoids walls and its own tail, and tracks your score and high score across sessions.The snake is represented by a deque of (x, y) grid coordinates.
It begins centered, moving to the right with a default length of 3 segments.
Movement is controlled via keyboard arrows, but direct reversal (turning 180° into itself) is disallowed.
Each move:Calculates the next cell based on the current direction.
Ends the game if the snake hits a wall or runs into itself (alive = false).
Otherwise, the snake’s head advances to the new cell. If it hasn't just eaten, the tail segment is removed; otherwise, the snake grows by keeping the tail.
 Food Management
Food appears at a random grid location where the snake doesn't occupy.
It avoids overlapping the snake and repositions after every successful meal.
# Score & High Score Handling
Every time the snake eats food:
The snake grows.
The score increases by one.
Game speed increases slightly (by 10% every 5 points), up to a capped max speed.
The high score is loaded from "highscore.txt" at launch and saved back if surpassed. A simple text file ensures persistence between plays.
#Graphics, Sound, and Textual Feedback
Graphics:
Green squares represent the snake (lighter for head, darker for body).
Red square shows food.
Text:
Score, High Score, and current speed are shown continuously at the top-left.
When the game is paused (with Space), a "PAUSED" overlay appears.
On game over, a clear "GAME OVER – Press R to Restart" message appears.
Sound:
Eating food triggers a “eat” sound (if eat.wav exists).
Game over plays a separate sound (gameover.wav), if available.
Background music (bgm.ogg) loops softly for ambiance and respects volume control via + / - keys.
#Game Loop Mechanics
The game runs in a while loop as long as the window is open.
A clock tracks elapsed time to regulate movement based on speed. Faster speed means shorter time between moves.
Event Processing:
Allows directional input, toggling pause, volume adjustments, grid visibility toggling, and restarting after game over (R key).
Updating:
If not paused and still alive, the snake moves and checks for food consumption or collisions.
Rendering:
It clears the screen, optionally draws a grid overlay, renders snake and food, overlays score and state messages, then displays the updated screen

#OUTPUT:-
![Image](https://github.com/user-attachments/assets/1173b901-f9dc-4784-8cf3-b0bcdaaec9e5)
