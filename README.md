# _IntroductionToRobotics Matrix Project_
# Snake : Snacks & Blocks  :snake: :            :apple: &  :bomb: 

<details>
  <summary> 
     <h2> Task Requirements :page_with_curl:  </h2>
  </summary>
  
  <details>
    <summary> 
     <h3>Menu Requirements :bookmark_tabs:</h3>
  </summary>
  
   Create a menu for your game, emphasis on the game. The player
  should scroll on the LCD with the joystick. The menu should include the following functionality:
  
  * When powering up a game, a greeting message should be shown for
  a few moments.
  
  * Should contain roughly the following categories:
  
    * Start game, starts the initial level of your game
    
    * Highscore:
    
      * Initially, we have 0.
      
      * Update it when the game is done. Highest possible score
    should be achieved by starting at a higher level.
    
      * Save the top 5+ values in EEPROM with name and score.
    
    * Settings:
      * Enter name. The name should be shown in highscore.
      
      * Starting level: Set the starting level value. The idea is to
      be able to start from a higher level as well. Can be replaced
      with difficulty.
      
      * LCD contrast control (optional, it replaces the potentiometer). Save it to eeprom.
      
      * LCD brightness control (mandatory, must change LED wire
      that’s directly connected to 5v). Save it to eeprom.
      
      * Matrix brightness control (see function setIntesnity from the
      ledControl library). Save it to eeprom.
      
      * Sounds on or off. Save it to eeprom.
      
      * Extra stuff can include items specific to the game mechanics,
      or other settings such as chosen theme song etc. Again, save
      it to eeprom.
      
    * About: should include details about the creator(s) of the game.
    At least game name, author and github link or user
    
    * How to play: short and informative description
    
   * While playing the game: display all relevant info
   
    * Lives
    
    * Level
    
    * Score
    
    * Time (optional)
    
    * Player name (optional)

  *  Upon game ending:
  
    * Screen 1: a message such as ”Congratulations on reaching level/score X”. ”You did better than y people.”. etc. Switches to screen 2 upon interaction (button press) or after a few moments.

    * Screen 2: display relevant game info: score, time, lives left etc. Must inform player if he/she beat the highscore. This menu should only be closed by the player, pressing a button.

  
</details>

  <details>
    <summary> 
     <h3>Game Requirements :bookmark_tabs:</h3>
  </summary>
  
  * Minimal components: an LCD, a joystick, a buzzer and the led matrix.
  
  * Must have basic sounds to the game (when ”eating” food, when dying, when finishing the level etc). Extra: add theme songs.
  
  * It must be intuitive and fun to play.
  
  * It must make sense in the current setup. Study the idea of a panning camera - aka the 8x8 led doesn’t need to be the entire map. It can only be the current field of view of the player.
  
  * The levels must progress dynamically. Aka the level difficulty, score and other properties should be a function of the level number or time. However, the challenge here is for it to grow in a playable way - not too easy for a long time so it becomes boring, not too hard too fast so it’s not attractive. Also, it’s nice to have an ending, a final level, a boss etc. It shouldn’t necessarily go on forever (but that’s fine, though).
  
  </details>
  
</details>
  
</details>

<details>
  <summary> 
    <h2>Components :electric_plug:</h2>
  </summary>
  
  * LED Matrix (8x8)
  
  * LCD Display (16x2)
  
  * Buzzer
  
  * Joystick
  
  * Potentiometer
  
  * Resistors and wires (per logic)
</details>

<details open>
  <summary> 
     <h2>Description :video_game:</h2>
  </summary>

  _Snake : Snacks & Bombs_ is a variation of the classic snake game with the addition of bombs and various difficulty levels. 
  
  * The main goal is to score as much as possible by collecting food, avoiding bombs, walls and the snake hitting itself. :snake: :apple: :bomb: 
  * The game has different levels of difficulty based on current level which imply higher speed or/and bombs. Bombs blink slower than food and they remain on the matrix until the snake touches them. The snake will lose a life each time it touches a bomb as an effect of the explosition. :bomb: :boom: :broken_heart:
  * When the snake has no lifes remaining, either by hiting three bombs, a wall or itself, it dies and the current game ends. If the player beats any of the top 5 highscore values he will enter the highscore top. :trophy:
</details>