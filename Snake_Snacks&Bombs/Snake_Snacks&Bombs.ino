// Matrix Menu & Demo for Game

/* Master Comment

# How to navigate through the code based on [CTRL + F]


# Logic

The game is controlled in the parseCurrState() function which, based on the current state (value of currState varibale) enters a corresponding function that controls the state
(e.g. for currState == MENU, enterMenu() function is entered and so on...).


*/

#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>
#include "Constants.h"

// [VARIABLES]

// control of the matrix and lcd display
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// states of switch and buzzer
byte swState = LOW,
     lastSwState = LOW,
     buzzerState = HIGH;

// initialize with true to reinitialize EEPROM
bool initEEPROM = true;

// universal variable used to temporarly store different addresses
int address;

// current matrix brightness
int matrixBrightness = 2;

// time of when the current game starts
unsigned long gameStartTime = 0;

// in-game variables for current number of lifes, current score, current number of minutes and seconds that have passed
int lifes = 3,
    currScore = -1,
    noOfMinutes = 0,
    noOfSeconds = 0;

// bitmap used to store location of bombs
byte bombsBitmap[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
};

// index of the current option for and total number of options for menus and submenus or game settings (difficulty, brightness, letter position when setting the name etc)
int diffIndex = 0, 
    aboutIndex = 1,
    settingsIndex = 1,
    howToIndex = 1,
    soundIndex = 1,
    menuIndex = 1,
    letterPos = 0,
    brightnessOptionIndex = 1;

// the current state we are in regarding functionality options
int currState = WELCOME;

// snake move and snake size
int snakeMove = UP,
    snakeSize = 3;

// arrays used to store snake components positions
int snakeRow[matrixSize * matrixSize],
    snakeCol[matrixSize * matrixSize];

// true if the snake changed size after eating
bool changedSize = false;

// time related variables related to the last time the snake moved, food / bomb was generated 
unsigned long lastMoveTime = 0,
              generatedFood = 0,
              generatedBomb = 0;

int switchPress = NONE,
    joystickMove = NONE,
    lastJoystickMove = NONE;

// passed short delay is used to detect a short press of the button
bool passedShortDelay = false,
     canScrollDown = true, 
     canScrollUp = false,
     joyMoved = false;

// last time a message has been displayed on lcd in display text function, last debounce time for switch press and last blink time for food
unsigned long lastTextLineMessageTime = -1,
              lastDebounceTime = 0,
              lastBlinkFoodTime = 0,
              lastBlinkBombTime = 0;

// variables for current position of the snake, food and currently generated bomb
int currRow = 0,
    currCol = 3,
    foodCol = 0,
    foodRow = 0,
    bombCol = 0,
    bombRow = 0;

// true if the snake touches a food / bomb object (matrix led)
bool collectedFood = false,
     explodedBomb = false;

// minimum and maximum thresholds for joystick movement coordinates
int minThreshold = 250,
    maxThreshold = 750;

// arrays of current name of the player and current names in Top 5
char currName[nameSize],
     highscoreNames[highscores][nameSize];

// array of current score values in Top 5
int highscoreValues[highscores];

// highscore variables for placement achieved and iteration
int placeInHighscoreTop = -1,
    highscoreIndex = highscores - 1,
    lcdBrightness = maxLcdBrightness;

// true if the game is in progress
bool inGame = true;

unsigned long moveInterval = 20;

// the time the function was called (for endGame function and displayFirstEndScreen function) or -1 if unset
unsigned long calledFirstEndScreen = -1,
              calledEndGame = -1;

// array of levels configuration
levelConfiguration levelsConfiguration[] = {{5, 700, 0, 1, 10000, 0},
                                            {20, 350, 0, 5, 10000, 0},
                                            {50, 550, 10000, 10, 5000, 1},
                                            {300, 300, 10000, 25, 5000, 1}};

void setup() {
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  
  // random seed in order for each game experience to be different
  randomSeed(analogRead(0));

  initializeLedMatrix();

  initializeLcdDisplay();

  if (initEEPROM) {
    initializeEEPROM();
  }
  else {
    setupFromEEPROM();
  }

  setNextState(WELCOME);
}

void loop() {
  parseCurrState();
}

// [STATE MANAGER] : each state represents either a menu or submenu option and they are switched by moving "back" or pressing to save, depending of the settings
void parseCurrState() {
  switch (currState) {
    case WELCOME:
      // display welcome message and correspoding matrix image
      enterWelcome();
      break;
    case MENU:
      // enters the menu of the game
      enterMenu();
      break;
    case START_GAME:
      // starts a new game
      enterGame();
      break;
    case SETTINGS:
      // enters the settings menu
      enterSettings();
      break;
    case ABOUT:
      // enters the about section
      enterAbout();
      break;
    case HOW_TO:
      // enters the how to play section
      enterHowTo();
      break;
    case HIGHSCORE:
      // enters the top highscores section
      enterHighscore();
      break;
    case END_GAME:
      // the game has ended
      endGame();
      break;
    case SET_SOUND:
      // the user sets sounds option (ON/OFF) from submenu 
      setSound();
      break;
    case SET_DIFFICULTY:
      // the user sets difficulty option (EASY/MEDIUM/HARD/INSANE) from submenu 
      setDifficulty();
      break;
    case SET_BRIGHTNESS:
      // the user sets which component's brightness to modify: matrix / lcd display
      setBrightness();
      break;
    case ENTER_NAME:
      // the user sets his name from the menu
      enterName();
      break;
    case SET_MATRIX_BRIGHTNESS:
      // the user sets matrix brightness from submenu option 
      setMatrixBrightness();
      break;
    case SET_LCD_BRIGHTNESS:
      // the user sets lcd brightness from submenu option
      setLcdBrightness();
      break;
    case RESET_HIGHSCORES:
      // resets all highscores from top to "??? 0" : unknown name and current score 0
      resetHighscores();
      break;
    case ENTER_NAME_FOR_HIGHSCORE:
      // the user sets his name because he didn't before starting the game and he reached a highscore
      enterName();
      break;
    case DISPLAY_FIRST_HIGHSCORE_MESSAGE:
      // displays first corresponding message after the game ended
      FirstEndScreen();
      break;
    case DISPLAY_SECOND_HIGHSCORE_MESSAGE:
      // displays second message after the first message displayed after game ended
      SecondEndScreen();
      break;
    case RESET_NAME:
      // resets name to "???", equivalent to name not being set
      resetNameSetting();
    default:
      break;
  }
}

// [WELCOME] : displays welcome message (which can be skipped with a press) and corresponding matrix image 
void enterWelcome() {
  const char welcomeText[][16] = {
    {'O', 'n', 'e', ' ', 'o', 'f', ' ', 't', 'h', 'e', ' ', 'b', 'e', 's', 't', ' '},
    {'g', 'a', 'm', 'e', 's', ' ', 'i', 's', ' ', 'b', 'a', 'c', 'k', '!', ' ', ' '},
    {'W', 'e', 'l', 'c', 'o', 'm', 'e', ' ', 't', 'o',  ' ', 'S', 'n', 'a', 'k', 'e'},
    {'S', 'n', 'a', 'c', 'k', 's', ' ', ' ', '&', ' ', 'B', 'o', 'm', 'b', 's', '!'},
    {'T', 'a', 'k', 'i', 'n', 'g', ' ', 'y', 'o', 'u', ' ', 't', 'o', ' ', ' ', ' '},
    {'t', 'h', 'e', ' ', 'm', 'e', 'n', 'u', ' ', '.', '.', '.', ' ', ' ', ' ', ' '},
  };

  displayImageOnMatrix(welcomeImage);
  displayScrollingText(welcomeText, welcomeTextSize, MENU);

  // welcome text can be skipped by pressing the button
  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    setNextState(MENU);
  }
} 

// [MENU] : displays menu options and corresponding matrix image; by pressing, the option display on the lcd will be selected and the user will enter its submenu
void enterMenu() {
  static const char menuText[][16] = {
    {' ', ' ', ' ', ' ', ' ', 'M', 'e', 'n', 'u', ' ', ' ', ' ', ' ', ' ', ' '},
    {'S', 't', 'a', 'r', 't', ' ', 'G', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'H', 'i', 'g', 'h', 's', 'c', 'o', 'r', 'e', 's', ' ', ' ', ' ', ' ', ' '},
    {'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'A', 'b', 'o', 'u', 't', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'H', 'o', 'w', ' ', 't', 'o', ' ', 'p', 'l', 'a', 'y', ' ', ' ', ' ', ' '},
  };

  displayImageOnMatrix(welcomeImage);

  scrollThrough(menuText, menuOptions, menuIndex, MENU, 1, menuOptions - 1, 1, 1, true);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    // by pressing the user choose the menu option displayed on the lcd
    parseMenuOption(menuIndex);
  }
}

// [MENU OPTION] :TO DO
void parseMenuOption(const int menuIndex) {
    lcd.clear();
    clearMatrix();

    switch(menuIndex) {
      case START_GAME:
        currState = START_GAME;
        gameStartTime = millis();
        lastJoystickMove = NONE;
        placeInHighscoreTop = -1;
        calledEndGame = -1;
        inGame = true;
        snakeSize = 3;
        for (int i = 0; i < snakeSize; i++) {
          snakeRow[i] = snakeStartRow[i];
          snakeCol[i] = snakeStartCol[i];
          lc.setLed(0, snakeCol[i], snakeRow[i], 1);
        }

        currRow = snakeRow[0];
        currCol = snakeCol[0];
        
        EEPROM.get(currDiffAddress, diffIndex);

        if(diffIndex == 0) {
          currScore = 0;
        }
        else {
          currScore = levelsConfiguration[diffIndex - 1].scoreThreshold;
        }

        lifes = 3;
        noOfMinutes = 0;
        noOfSeconds = 0;
        snakeMove = UP;
        changedSize = false;
        collectedFood = false;
        lastMoveTime = millis();
        generatedFood = millis();

        for (int i = 0; i < matrixSize; i++) {
          for (int j = 0; j < matrixSize; j++) {
            bombsBitmap[i][j] = 0;
          }
        }
        break;
      case HIGHSCORE:
        //readHighscores();
        //address = highscoreStartingAddress;
        setNextState(HIGHSCORE);
        break;
      case SETTINGS:
        setNextState(SETTINGS);
        break;
      case ABOUT:
        setNextState(ABOUT);
        break;
      case HOW_TO:
        setNextState(HOW_TO);
        break;
      // case 6:
      //   currState = RESET_HIGHSCORES;
      //   break;
      default:
        break;
    }
}

// [GAME] : snake movement, food +/- bombs generation, display of lcd game information and control of reaching next level / game end
void enterGame() { 
  // controls snake movement during the game
  inGameMovement();

  // generates food (one fast blinking led on the matrix)
  generateFood();

  // if the level configuration implies bombs they will be generated (multiple slower blinking leds on the matrix)
  if (levelsConfiguration[diffIndex].spanBombs) {
    generateBombs();
  }
  
  displayLcdGame();  

  // current game ends if life number reaches 0 
  if (lifes == 0) {
    beep(BEEP_LOW, BEEP_DURATION);
    inGame = false;
    setNextState(END_GAME);
  }

  // if the current score surpasses the current level threshold the player will be taken to the next difficulty or the game will end if the current difficulty was the highest
  if (currScore >= levelsConfiguration[diffIndex].scoreThreshold) {
    // current difficulty was the highest, therefore game will end
    if (diffIndex == diffOptions - 1) {
      lcd.clear();
      lcd.print(diffIndex);
      currState = END_GAME;
    }
    else {
      // continuing and informing the player by playing a high sound that he reached the next difficulty
      diffIndex++;
      beep(BEEP_HIGH, BEEP_DURATION);
    }
  }
}

// [LCD GAME] : displays all lcd game current information regarding lifes, score, timer, name, difficulty
void displayLcdGame() {
  displayLife();
  displayScore();
  displayTimer();
  displayName();
  displayDifficulty();
}

// displays current number of lifes and suggestive character resembling a heart
void displayLife() {
  lcd.setCursor(0,0);
  lcd.write(CHAR_HEART);
  lcd.setCursor(2,0);
  lcd.print(digits[lifes]);
}

// displays current score and suggestive character resembling a star
void displayScore() {
  lcd.setCursor(4,0);
  lcd.write(CHAR_STAR);

  lcd.setCursor(6,0);

  if (currScore < 10) {
    lcd.print(digits[currScore]);
  }
  else if(currScore < 100){
    lcd.print(digits[currScore / 10]);
    lcd.print(digits[currScore % 10]);
  }
  else
  {
    lcd.print(digits[currScore / 100]);
    lcd.print(digits[(currScore / 10) % 10]);
    lcd.print(digits[currScore % 10]);
  }
}

// displays the current timer (how much time has passed since game start) and suggestive character resembling a clock 
void displayTimer() {
  // TO DO: make noOfSeconds & noOfMinutes static?

  lcd.setCursor(10,0);
  lcd.write(CHAR_CLOCK);
  lcd.setCursor(11,0);

  noOfMinutes = (millis() - gameStartTime) / 60000;

  // prints minutes including necessary padding if the number has only one digit
  if (noOfMinutes < 10) {
    lcd.print("0");
    lcd.setCursor(12,0);
  }

  lcd.print(noOfMinutes);
  lcd.print(":");
  lcd.setCursor(14,0);

  // calculates number of seconds that have passed
  noOfSeconds = (millis() - gameStartTime) / 1000 -  60 * noOfMinutes;

  // when 60 seconds have passed a minute is added
  if (noOfSeconds >= 60) {
    noOfMinutes++;
    noOfSeconds -= 60;
  }

  // prints seconds including necessary padding if the number has only one digit
  if (noOfSeconds < 10) {
    lcd.print("0");
  }
	
  lcd.print(noOfSeconds);
}

// displays name during game
void displayName() {
  lcd.setCursor(0,1);
  lcd.write(CHAR_HUMAN);
  lcd.print(" ");

  printCurrName();
}

// displays current difficulty during game
void displayDifficulty() {
  lcd.setCursor(8,1); 
  lcd.print("  "); 
  lcd.setCursor(10,1);
  lcd.print(difficulty[diffIndex]);
}

// [UNIVERSAL FUNCTIONS]

void initializeLcdDisplay() {
  lcd.begin(16, 2);
  createCustomChars();
}

void initializeLedMatrix() {
  lc.shutdown(0, false);
  lc.clearDisplay(0);
}

void createCustomChars() {
  lcd.createChar(CHAR_DOWN_ARROW, charDownArrow);
  lcd.createChar(CHAR_UP_ARROW, charUpArrow);
  lcd.createChar(CHAR_HEART, charHeart);
  lcd.createChar(CHAR_CLOCK, charClock);
  lcd.createChar(CHAR_STAR, charStar);
  lcd.createChar(CHAR_HUMAN, charHuman);
}

// [EEPROM] : initialises the EEPROM by setting default values to the current name, top 5 highscores, difficulty, sound, lcd and matrix brightness
void initializeEEPROM() {
  // current name 
  resetName();

  // highscores
  resetHighscores();

  // difficulty
  diffIndex = 0;
  EEPROM.put(currDiffAddress, diffIndex);

  // sound
  buzzerState = HIGH;
  EEPROM.put(soundAddress, HIGH);

  // led brightness
  EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  setLcdBrightnessFromEEPROM();

  // matrix brightness
  EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  setMatrixBrightnessFromEEPROM();
}

// [EEPROM setup] : sets up values for current name, top 5 highscores, difficulty, sound, lcd and matrix brightness by accessing the EEPROM addresses where the values are stored at
void setupFromEEPROM() {
  setNameFromEEPROM();

  setHighscoreTopFromEEPROM();
    
  setDifficultyFromEEPROM();
  
  setSoundFromEEPROM();

  setLcdBrightnessFromEEPROM();

  setMatrixBrightnessFromEEPROM();
}

// sets current name by accessing EEPROM address where value is stored
void setNameFromEEPROM() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    address += sizeof(char);
  }
}

// sets current top 5 highscores by accessing EEPROM address where values are stored
void setHighscoreTopFromEEPROM() {
  address = highscoreStartingAddress;
    
  for (int i = 0; i < highscores; i++) {
    for (int j = 0; j < nameSize; j++) {
      EEPROM.get(address, highscoreNames[i][j]);
      address += sizeof(char);
    }
  }

  for (int i=0; i < highscores; i++) {
    EEPROM.get(address, highscoreValues[i]);
    address += sizeof(int);
  }
}

// sets current difficulty by accessing EEPROM address where value is stored
void setDifficultyFromEEPROM() {
  EEPROM.get(currDiffAddress, diffIndex);
}

// sets current sound option by accessing EEPROM address where value is stored
void setSoundFromEEPROM() {
  EEPROM.get(soundAddress, buzzerState);
}

// sets current lcd brightness by accessing EEPROM address where value is stored
void setLcdBrightnessFromEEPROM() {
  EEPROM.get(lcdBrightnessAddress, lcdBrightness);
  analogWrite(lcdBacklightPin, lcdBrightness * LCD_BRIGHTNESS_FACTOR);
}

// sets current matrix brightness by accessing EEPROM address where value is stored
void setMatrixBrightnessFromEEPROM() {
  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  lc.setIntensity(0, matrixBrightness); 
}

/* [SCROLLING TEXT] : universal function to display scrolling text letter by letter at scrollInterval on two rows (used for welcome text) in the current context it is used only for the welcome message
   parameters : text (represented by a char matrix), number of lines and the state to transition to after the whole message has been displayed */
void displayScrollingText(const char text[][16], const int noOfLines, const int nextState) {
  static int scrollInterval = 175,
             lineIndex = 0,
             columnIndex = 0,
             cursorRow = 0;

  // initialize last message time at first call of the function if unset in order to keep track of scroll interval per row
  if (lastTextLineMessageTime == -1 && millis() != 0) {
    lcd.setCursor(0, cursorRow);
    lastTextLineMessageTime = millis();
  }

  // if the scrolling interval has been reached another letter is displayed
  if (millis() - lastTextLineMessageTime >= scrollInterval && columnIndex < 17) {
    lcd.print(text[lineIndex][columnIndex]);
    lastTextLineMessageTime = millis();
    columnIndex += 1;
  }
  
  // maximum column index has been reached surpassed; if the whole message has been displayed, transition to next state, otherwise, transition to the other cursor row 
  if (columnIndex > maxColValueLcd) { 
    // reinitialize last message time, column index and line index
    lastTextLineMessageTime = -1;
    columnIndex = 0;
    lineIndex += 1;

    // the whole message has been displayed and the next step is to transition to the next state
    if (lineIndex == noOfLines) {
      setNextState(nextState);
    }

    // reset row to the other cursor row value because the maximum column index has been reached on the current value
    if (cursorRow == 1) {
      lcd.clear();
      cursorRow = 0;
    }
    else {
      cursorRow = 1;
    }
  }
}

// [NEXT STATE] : transitions to the next state but clearing the lcd display plus resetting the cursor to top left position and changing the current state to the next state
void setNextState(const int nextState) {
  lcd.clear();
  lcd.setCursor(0,0);
  currState = nextState;
}

// [MATRIX IMAGE] : displays image on led matrix
void displayImageOnMatrix(uint64_t image) {
  for (int i = 0; i < matrixSize; i++) {
    byte row = (image >> i * matrixSize) & 0xFF;
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}

// [CLEAR MATRIX] : shuts off all matrix leds clearing the matrix image
void clearMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, 0);
    }
  }
}

// [BEEP] : tones the buzzer for a limited amount of time resembling a "beep" sound
void beep(const int toneValue, const int timeLength) {
  if (buzzerState) {
    tone(buzzerPin, toneValue, timeLength);
  }
}

// [JOYSTICK] : returns current joystick move : NONE / UP / DOWN / LEFT
int getJoystickMove() {
  static int xValue = 0,
             yValue = 0,
             xChange = 0,
             yChange = 0;

  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  xChange = abs(initialXValue - xValue);
  yChange = abs(initialYValue - yValue);

  if (!joyMoved) {     
    if (yChange >= xChange) {
      if (yValue < minThreshold) {
        joyMoved = true;
        return UP;
      }
      if (yValue > maxThreshold) {
        joyMoved = true;
        return DOWN;
      }
    } 
    else {
      if (xValue < minThreshold) {
        joyMoved = true;
        return LEFT;
      }
      if (xValue > maxThreshold) {
        joyMoved = true;
        return RIGHT;
      }
    }
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }

  return NONE;
}

// [SWITCH] : based on pressing history, returns corresponding type of press (SHORT / LONG) or NONE if absent
int getSwitchPress() {
   int reading = digitalRead(pinSW);

    if (reading != lastSwState) {
      lastDebounceTime = millis();
    }

    lastSwState = reading;

    if (millis() - lastDebounceTime >= debounceDelay) {
      if (reading != swState) {
        swState = reading;

        if (swState == LOW) {
          passedShortDelay = true;
        }
      }

      if (swState == LOW) {
        if (millis() - lastDebounceTime >= debounceDelayLong) {
          return LONG_PRESS;
        } 
      } 
      else {
        if (passedShortDelay) {
          passedShortDelay = false;
          return SHORT_PRESS;
        }
      }
    }

    return NONE;
}

// [DISPLAY ARROWS] : display arrows based on the current values of canScrollUp and canScrollDown
void displayArrows() {
  // up arrow
  lcd.setCursor(15,0);
  if (canScrollUp) {
    lcd.write(CHAR_UP_ARROW);
  }
  else
  {
    lcd.print(" ");
  }

  //down arrow
  lcd.setCursor(15,1);
  if (canScrollDown) {
    lcd.write(char(CHAR_DOWN_ARROW));
  }
  else
  {
    lcd.print(" ");
  }
}

// TO DO
// [SCROLL] : universal function that scrolls through text options with the possibility of saving a certain setting
void scrollThrough(const char matrix[][16], const int options, int &scrollingIndex, const int returnToState, const int lowerBoundIndex, const int upperBoundIndex, const int upperBoundCursorRow, const int initialScrollingIndex, bool automaticSave){
  if (!automaticSave) {
    lcd.setCursor(0,0);
    lcd.print(matrix[scrollingIndex]);
    lcd.setCursor(0,1);
    lcd.print("Press to save.");
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print(matrix[0]);
    lcd.setCursor(0,1);
    lcd.print(matrix[scrollingIndex]);
  }

  displayArrows();

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
    lcd.clear();
    beep(BEEP_HIGH, BEEP_DURATION);
    scrollingIndex = max(scrollingIndex - 1, lowerBoundIndex);
  }  
  else if (joystickMove == RIGHT) {
    lcd.clear();
    beep(BEEP_HIGH, BEEP_DURATION);
    scrollingIndex  = min(scrollingIndex + 1, upperBoundIndex);
  }
  else if (joystickMove == DOWN && automaticSave) {
    scrollingIndex = lowerBoundIndex;
    beep(BEEP_HIGH, BEEP_DURATION);
    setNextState(returnToState);
  }

  canScrollUp = !(scrollingIndex == lowerBoundIndex);
  canScrollDown = !(scrollingIndex == upperBoundIndex);
}

// [BLINK FOOD] : blinks the food led by taking its position as parameter and the blinking interval
void blinkFood (const int col, const int row, const int blinkInterval) {
  static byte blinkState = 1;

  if (millis() - lastBlinkFoodTime >= blinkInterval) {
    blinkState = !blinkState;
    lc.setLed(0, col, row, blinkState);
    lastBlinkFoodTime = millis();
  }
}

// [BLINK BOMBS] : blinks the bombs currently on the led matrix (information stored in the bombs bitmap)
void blinkBombs (const int blinkInterval) {
  static byte blinkState = 1;

  // blinking all the bombs currently on the led matrix
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      // check if the bomb is currently on the led matrix
      if (bombsBitmap[i][j]) {
        lc.setLed(0, j, i, blinkState);
      }
    }
  }

  // every time blinkInterval passes blink state will change to its opposite value to create the blinking effect by alternating HIGH and LOW states
  if (millis() - lastBlinkBombTime >= blinkInterval) {
    blinkState = !blinkState;
    lastBlinkBombTime = millis();
  }
}

// [GENERATE FOOD] : generates food (one fast blinking led on the matrix) on a random available position on the matrix; controls its blinking
void generateFood() {
  static const int blinkInterval = 100;
  
  static bool expiredFoodTime = false;
  static int intervalMultiplier = 5000;
  static unsigned long foodOnScreenInterval = 0;
  static bool badFood = true;

  // depending on difficulty, the foodOnScreenInterval is either loger or shorter
  foodOnScreenInterval = levelsConfiguration[diffIndex].foodSpanInterval;

  if (inGame) {
    if (collectedFood || currScore == -1 || expiredFoodTime) {
      collectedFood = false;
      lastBlinkFoodTime = millis();
      generatedFood = millis();
      // food disapears after foodOnScreenInterval time
      expiredFoodTime = false;

      // set the current score if unset
      if (currScore == -1) currScore = 0;

      // avoid the case when the food spawns on the current position of the player or of a bomb, could lead to confusion / increased difficulty
      badFood = true;
      while (badFood) {
        badFood = false;
        // generate random position on the led matrix
        foodCol = rand() % matrixSize, foodRow = rand() % matrixSize;
        // check if the food would spawn on the snake
        for (int i = 0; i < snakeSize; i++) {
          if (foodCol == snakeCol[i] && foodRow == snakeRow[i]) {
            badFood = true;
          }
        }
        // check if the food would spawn on a bomb
        if (bombsBitmap[foodRow][foodCol] == 1) {
          badFood = true;
        }
      }
    }

    // if the time the food is to stay on the matrix is up, the food will dissapear and respawn
    if (millis() - generatedFood >= foodOnScreenInterval) {
      expiredFoodTime = true;
      lc.setLed(0, foodCol, foodRow, 0);
    }

    blinkFood(foodCol, foodRow, blinkInterval);
  }
}

// [GENERATE BOMBS] : generates bombs (multiple slower blinking leds on the matrix) on a random available position on the matrix; controls their blinking
void generateBombs() {
  static const int blinkInterval = 700;
  static int intervalMultiplier = 7000;
  static bool generateNewBomb = false;
  static unsigned long generateBombsInterval = 0;

  // depending on difficulty, the foodOnScreenInterval is either loger or shorter
  generateBombsInterval = levelsConfiguration[diffIndex].spanBombsInterval;

  if (inGame) {
    if (generateNewBomb) {
      generatedBomb = millis();
      generateNewBomb = false;

      // avoid the case when the bomb spawns on the current position of the player or of the food, could lead to confusion
      bool badBomb = true;
      while (badBomb) {
        badBomb = false;
        // generates random position on the led matrix
        bombCol = rand() % matrixSize, bombRow = rand() % matrixSize;
        for (int i = 0; i < snakeSize; i++) {
          // check if the bomb would spawn on the snake
          if (bombCol == snakeCol[i] && bombRow == snakeRow[i]) {
            badBomb = true;
          }
        }
        // check if the bomb would spawn on the food
        if(bombCol == foodCol && bombRow == foodRow){
          badBomb = true;
        }
        // check if the bomb would spawn on another bomb
        if(bombsBitmap[bombRow][bombCol] == 1) {
          badBomb = true;
        }
      }

      // marks the bomb on the bitmap
      bombsBitmap[bombRow][bombCol] = 1;
    }

    // the interval to generate new bomb has passed
    if (millis() - generatedBomb >= generateBombsInterval) {
      generateNewBomb = true;
    }

    blinkBombs(blinkInterval);
  }
  
}

// [GAME MOVEMENTS] : controls snake movement during the game TO DO
void inGameMovement() {
  static const int intervalMultiplier = 200;

  joystickMove = getJoystickMove();

  if (joystickMove != NONE) {
    // if the snake is moving vertically he can only go in another horizontal direction
    if ((snakeMove == UP || snakeMove == DOWN) && (joystickMove == LEFT || joystickMove == RIGHT)) {
      snakeMove = joystickMove;
    }
    // if the snake is moving horizontally he can only go in another vertical direction
    else if ((snakeMove == LEFT || snakeMove == RIGHT) && (joystickMove == UP || joystickMove == DOWN)) {
      snakeMove = joystickMove;
    }
  }

  if (millis() - lastMoveTime > levelsConfiguration[diffIndex].levelSpeed) { 
    lastMoveTime = millis();

    switch(snakeMove){
      case LEFT:
        // if it touches a wall the snake instantly dies
        if (currCol == 0) {
          lifes = 0;
        }
        currCol = max(currCol - 1, 0);
        break;
      case RIGHT:
        // if it touches a wall the snake instantly dies
        if (currCol == matrixSize - 1) {
          lifes = 0;
        }
        currCol = min(currCol + 1, matrixSize - 1);
        break;
      case UP:
        // if it touches a wall the snake instantly dies
        if (currRow == matrixSize - 1) {
          lifes = 0;
        }
        currRow = min(currRow + 1, matrixSize - 1);
        break;
      case DOWN:
        // if it touches a wall the snake instantly dies
        if (currRow == 0) {
          lifes = 0;
        }
        currRow = max(currRow - 1, 0);
        break;
    }

    // if the snake doesn't change size, turn off last led as it is moving forward 
    if (!changedSize) {
      lc.setLed(0, snakeCol[snakeSize-1], snakeRow[snakeSize-1], 0);
    }

    // reset change size
    changedSize = false;

    // moving the snake
    for (int i = snakeSize - 1; i > 0; i--) {
      snakeRow[i] = snakeRow[i-1];
      snakeCol[i] = snakeCol[i-1];
    }

    // reinitialize the head of the snake with the current position and light up the led
    snakeRow[0] = currRow;
    snakeCol[0] = currCol;
    lc.setLed(0, currCol, currRow, 1);

    // if the snake touches itself the game instantly ends (lifes reaches zero) as the snake dies
    for (int i = 1; i < snakeSize; i++){
      if (snakeRow[i] == currRow && snakeCol[i] == currCol) {
        lifes = 0;
      }
    }

    // the snake collected the food by reaching its position
    if (currCol == foodCol && currRow == foodRow) {
      beep(BEEP_HIGH, BEEP_DURATION);

      collectedFood = true;

      // increase the current score with the food value specific to the current difficulty
      currScore += levelsConfiguration[diffIndex].foodValue;

      // increase the size of the snake that has changed
      snakeSize += 1;
      changedSize = true;
    }


    // the snake exploded a bomb by reaching its position
    if (bombsBitmap[currRow][currCol]) {
      beep(BEEP_LOW, BEEP_DURATION);

      // the bomb will no longer exist on the matrix since it exploded
      bombsBitmap[currRow][currCol] = 0;

      // decrease the current number of lifes by one
      lifes--;
    }
  }
}

// [ABOUT] : displays about text and the user can navigate through it
void enterAbout() {
  static const char aboutText[][16] = {
    {' ', ' ', ' ', ' ', ' ', 'A', 'b', 'o', 'u', 't', ' ', ' ', ' ', ' ', ' '},
    {'N', 'a', 'm', 'e', ':', ' ', 'S', 'n', 'a', 'k', 'e', ' ', ' ', ' ', ' '},
    {'S', 'n', 'a', 'c', 'k', 's', ' ', '&', ' ', 'B', 'o', 'm', 'b', 's', ' '},
    {'A', 'u', 't', 'h', 'o', 'r', ':', ' ', 'I', 'o', 'a', 'n', 'a', ' ', ' '},
    {'L', 'i', 'v', 'i', 'a', ' ', 'P', 'o', 'p', 'e', 's', 'c', 'u', ' ', ' '},
    {'G', 'i', 't', 'h', 'u', 'b', ' ', 'U', 's', 'e', 'r', 'n', 'a', 'm', 'e'},
    {'I', 'o', 'a', 'n', 'a', 'L', 'i', 'v', 'i', 'a', ' ', ' ', ' ', ' ', ' '},
  };

  displayImageOnMatrix(aboutImage);
  scrollThrough(aboutText, aboutOptions, aboutIndex, MENU, 1, aboutOptions - 1, 1, 1, true);
}

// [HOW TO] : displays how to play text and the user can navigate through it
void enterHowTo() {
  static const char howToMatrix[][16] = {
    {' ', ' ', 'H', 'o', 'w', ' ', 't', 'o', ' ', 'p', 'l', 'a', 'y', ' ', ' '},
    {'C', 'o', 'l', 'l', 'e', 'c', 't', ' ', 'f', 'o', 'o', 'd', '!', ' ', ' '},
    {'U', 's', 'e', ' ', 'j', 'o', 'y', 's', 't', 'i', 'c', 'k', ' ', 't', 'o'},
    {'m', 'o', 'v', 'e', ' ', 'W', 'A', 'S', 'D', '.', ' ', ' ', ' ', ' ', ' '},
    {'A', 'v', 'o', 'i', 'd', ' ', 'w', 'a', 'l', 'l', 's', ' ', 'a', 'n', 'd'},
    {'b', 'o', 'm', 'b', 's', ' ', 'w', 'h', 'i', 'c', 'h', ' ', 'b', 'l', 'i'},
    {'n', 'k', ' ', 's', 'l', 'o', 'w', 'e', 'r', '!', ' ', ' ', ' ', ' ', ' '},
    {'H', 'a', 'v', 'e', ' ', 'S', 's', 'o', 'm', 'e', ' ', 'F', 'u', 'n', '!'},
  };

  displayImageOnMatrix(howToImage);
  scrollThrough(howToMatrix, howToOptions, howToIndex, MENU, 1, howToOptions - 1, 1, 1, true);
}

// [PRINTS CURRENT NAME] : prints current name on lcd from EEPROM
void printCurrName() {
  address = currNameStartingAddress;
  
  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    lcd.print(currName[i]);
    address += sizeof(char);
  }
}

// [HIGHSCORE] : controls the highscores section
void enterHighscore() {
  displayImageOnMatrix(highscoreImage);

  lcd.setCursor(0,0);
  lcd.print("Highscores");
  lcd.setCursor(0,1);

  displayTopHighscores();

  displayArrows();

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
     highscoreIndex = min(highscoreIndex + 1, highscores - 1);
  }  
  else if (joystickMove == RIGHT) {
    highscoreIndex = max(highscoreIndex - 1, 0);
  }
  else if (joystickMove == DOWN) {
    beep(BEEP_HIGH, BEEP_DURATION);

    // reset highscore index to highest index for next scroll through highscores
    highscoreIndex = highscores - 1;
    setNextState(MENU);
  }

  canScrollUp = !(highscoreIndex == highscores - 1);
  canScrollDown = !(highscoreIndex == 0);
}

// displays top highscores : names and values
void displayTopHighscores() {
  for (int j = 0; j < nameSize; j++) {
    lcd.print(highscoreNames[highscoreIndex][j]);
  }

  lcd.print(" ");
  displayNumber(highscoreValues[highscoreIndex], 1, 5);
}

// [DISPLAY NUMBER] : universal display number function
void displayNumber(const int number, const int cursorRow, int cursorColumn) {
  lcd.setCursor(cursorColumn, cursorRow);

  if (number >= 100) {
   lcd.print(digits[(int)number / 100]);
  }
  else
  {
    lcd.print(" ");
  }
	
  if (number >= 10) {
    lcd.print(digits[(number / 10) % 10]);
  }
  else
  {
    lcd.print(" ");
  }
	
  cursorColumn += 2;
  lcd.print(digits[number % 10]);
  lcd.setCursor(cursorColumn + 1, cursorRow);
}

// [SETTINGS] : controls settings section
void enterSettings() {
  static const char settingsText[][16] = {
    {' ', ' ', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' '},
    {'E', 'n', 't', 'e', 'r', ' ', 'N', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'R', 'e', 's', 'e', 't', ' ', 'N', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'B', 'r', 'i', 'g', 'h', 't', 'n', 'e', 's', 's', ' ', ' ', ' ', ' ', ' '},
    {'S', 'o', 'u', 'n', 'd', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'D', 'i', 'f', 'f', 'i', 'c', 'u', 'l', 't', 'y', ' ', ' ', ' ', ' ', ' '},
    {'R', 'e', 's', 'e', 't', ' ', 'T', 'o', 'p', ' ', '5', ' ', ' ', ' ', ' '},
  };

  displayImageOnMatrix(settingsImage);
  scrollThrough(settingsText, settingsOptions, settingsIndex, MENU, 1, settingsOptions - 1, 1, 1, true);
  
  // by pressing the switch the current settings option displayed will be selected
  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    lcd.clear();
    parseSettingsOption(settingsIndex);
  }
}

// [SETTINGS OPTION] : settings option manager
void parseSettingsOption(const int settingsIndex) {
  switch(settingsIndex) {
    case 1:
      if (nameNotSet())
      {
        initializeName();
      }
      setNextState(ENTER_NAME);
      break;
    case 2:
      setNextState(RESET_NAME);
      break;
    case 3:
      setNextState(SET_BRIGHTNESS);
      break;
    case 4:
      setNextState(SET_SOUND);
      break;
    case 5:
      setNextState(SET_DIFFICULTY);
      break;
    case settingsOptions - 1:
      displayResetHighscoresMessage();
      setNextState(RESET_HIGHSCORES);
      break;
  }
}

// [INITIALIZE NAME] : initialises name with 'aaa' , lcd display and letter position for name to be set 
void initializeName() {
  for (int i = 0; i < nameSize; i++) {
        currName[i] = 'a';
  }

  saveName();

  lcd.clear();
  lcd.setCursor(0,0);
  letterPos = 0;
}

// [SAVE NAME] : saves current name in EEPROM
void saveName() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.put(address, currName[i]);
    address += sizeof(char);
  }

}

// [HIGHSCORES RESET MESSAGE] : displays message that lets the user know the top 5 highscores names and values have been reset
void displayResetHighscoresMessage() {
  lcd.setCursor(0,0);
  lcd.print("Top 5 highscores");
  lcd.setCursor(0,1);
  lcd.print("have been reset!");
}

/*[RESET HIGHSCORES]: resets values at corresponding memory addresses for the top highscores with '??? 0' each and updates highscoreNames and highscoreValues arrays that store current top highscores
"??? 0" corresponds to unknown name and a current score of 0 */
void resetHighscores() {
  displayResetHighscoresMessage();

  // reinitialize top highscores to EEPROM
  address = highscoreStartingAddress;
    
  for (int i = 0; i < highscores; i++) {
    for (int j = 0; j < nameSize; j++) {
      EEPROM.put(address, char('?'));
      address += sizeof(char);
    }
  }
  
  for (int i = 0; i < highscores; i++) {
    EEPROM.put(address, int(0));
    address += sizeof(int);
  }

  // reads from EEPROM updated values of highscores
  readHighscores();

  // go back to settings if selected by moving to the left ("Back")
  joystickMove = getJoystickMove();
	
  if (joystickMove == DOWN) {
    beep(BEEP_HIGH, BEEP_DURATION);
    setNextState(SETTINGS);
  }
}

// [READ HIGHSCORES] : read from EEPROM updated values of top highscores
void readHighscores() {
  address = highscoreStartingAddress;
    
  for (int i = 0; i < highscores; i++) {
    for (int j = 0; j < nameSize; j++) {
      EEPROM.get(address, highscoreNames[i][j]);
      address += sizeof(char);
    }
  }
  
  for (int i = 0; i < highscores; i++) {
    EEPROM.get(address, highscoreValues[i]);
    address += sizeof(int);
  }
    
}

// [SAVE HIGHSCORES] : save to EEPROM current values of top highscores
void saveHighscores() {
  address = highscoreStartingAddress;
    
  for (int i = 0; i < highscores; i++) {
    for (int j = 0; j < nameSize; j++) {
      EEPROM.put(address, highscoreNames[i][j]);
      address += sizeof(char);
    }
  }
  
  for (int i = 0; i < highscores; i++) {
    EEPROM.put(address, highscoreValues[i]);
    address += sizeof(int);
  }
}

// [RESET NAME SETTING] : manages reset name setting display message and functionality
void resetNameSetting() {
  displayResetNameMessage();

  // press to reset name
  if(getSwitchPress() != NONE) {
    resetName();
    setNextState(SETTINGS);
  }

  // go back to previous state
  joystickMove = getJoystickMove();

  if (joystickMove == DOWN) {
    beep(BEEP_HIGH, BEEP_DURATION);
    setNextState(SETTINGS);
  }
}

void displayResetNameMessage() {
  lcd.setCursor(0,0);
  lcd.print("New name: ???");
  lcd.setCursor(0,1);
  lcd.print("Press to save.");
}

// [RESET NAME]:  function that resets name to its before uploading game value which is "???" in both variable currName and EEPROM (unset; after setting a name it will never be able to gack to "???" unless reset from settings)
void resetName() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.put(address, char('?'));
    address += sizeof(char);
  }

  setNameFromEEPROM();
}


// [END GAME] : manages end game state, displays corresponding messages
void endGame() {
  static const int secondScreenMessageDelay = 3000;
  // if last time end game was called is unset ( == -1) is set to current time (millis()) in order to display second message after a second screen message delay has passed
  if (calledEndGame == -1) { 
    displayImageOnMatrix(endGameImage);
    calledEndGame = millis();
  }

  // if the second screen message delay has passed
  if (millis() - calledEndGame > secondScreenMessageDelay && calledEndGame != -1) { 
    calledFirstEndScreen == -1;

    // if the player got in the top 5 highscores by beating at least one of the values in the top
    if (placeInHighscoreTop != -1) {
      beep(BEEP_HIGH, BEEP_DURATION);
    }
    else
    {
      beep(BEEP_LOW, BEEP_DURATION);
    }
    
    setNextState(DISPLAY_SECOND_HIGHSCORE_MESSAGE);
  }
  else
  { 
    FirstEndScreen();
  }
}

// [FIRST END SCREEN] : manages first screen after game end
void FirstEndScreen() {
  // if the last time the first end screen function was called is unset it will be setted to currrent time
  if (calledFirstEndScreen == -1) {
    calledFirstEndScreen = millis();
  }

  displayFirstEndScreenMessage();

  // if the player got in the top 5 highscores the top will be updated
  if (placeInHighscoreTop == -1) {
    updateHighscores();
  }
}

void displayFirstEndScreenMessage() {
  lcd.setCursor(0,0);
  lcd.print("Congrats!");
  lcd.setCursor(13,0);

  printCurrName();

  lcd.setCursor(0,1);
  lcd.print("Level: ");
  displayNumber(diffIndex, 1, 9);
  lcd.print("!");
}

// [UPDATE HIGHSCORE TOP] : updated the highscores top based on the score obtained in the game that just has ended
void updateHighscores() {
  // checks if the current score is higher than any of the values in the top is descending order to find the correct insertion position
  for (int i = highscores - 1; i >= 0; i--) {
    if (currScore > highscoreValues[i] && currScore != 0) {
      placeInHighscoreTop = i;
      break;
    }
  }

  // if the current player scored a highscore and they set their name beforehand
  if (placeInHighscoreTop != -1 && currName[0] != '?') {

    for (int i = 0; i < placeInHighscoreTop; i++) {
      for (int j = 0; j < nameSize; j++) {
        highscoreNames[i][j] = highscoreNames[i + 1][j];
      }
      highscoreValues[i] =  highscoreValues[i + 1];
    }

    highscoreValues[placeInHighscoreTop] = currScore;

    for (int j = 0; j < nameSize; j++) {
      highscoreNames[placeInHighscoreTop][j] = currName[j];
    }

    saveHighscores();
  }
}

// [SECOND END SCREEN]
void SecondEndScreen() {
  lcd.setCursor(0,0);
  lcd.print("Your scored:");
  displayNumber(currScore, 0, 13);

  if (placeInHighscoreTop != -1) {
    address = currNameStartingAddress;

    for (int i = 0 ; i < nameSize; i++) {
        EEPROM.get(address, currName[i]);
        lcd.print(currName[i]);
        address += sizeof(char);
    }
    
    lcd.setCursor(0,1);
    displayImageOnMatrix(highscoreImage);
    lcd.print("is in Top 5 !!!");
  }
  else
  { 
    lcd.setCursor(0,1);
    displayImageOnMatrix(sadImage);
    lcd.print("...");
    lcd.setCursor(0,1);
    lcd.print("Not a highscore...");
  }

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    lcd.clear();
    lc.setLed(0, currRow, currCol, 0);
    lc.setLed(0, foodRow , foodCol, 0);
    resetGameVariables();

    if (placeInHighscoreTop != -1) {
      if (nameNotSet()) {
        initializeName();
        setNextState(ENTER_NAME_FOR_HIGHSCORE);
      }
      else {
        currScore = 0;
        setNextState(HIGHSCORE);
      }
    }
    else
    {
      currScore = 0;
      setNextState(MENU);
    }
  }
}

// [NAME UNSET] : return true is name is not set ("???") and false is set (!= "???")
bool nameNotSet() {
  return currName[0] == '?';  
}

// [RESET GAME VARIABLES] 
void resetGameVariables() {
  currCol = 0;
  currRow = 0;
  foodCol = 0;
  foodRow = 0;
  calledFirstEndScreen == -1;
  gameStartTime = 0;
  lifes = 3;
  noOfMinutes = 0;
  noOfSeconds = 0;
}

// [ENTER NAME] : manages enter name setting TO DO MORE COMMENTS
void enterName() {
  lcd.setCursor(0, 1);
  lcd.print("Press to save.");
  lcd.setCursor(0, 0);

  for (int i = 0; i < nameSize; i++) {
    lcd.print(currName[i]);
  }

  lcd.setCursor(9,0); 
  lcd.print("Len:3");
  displayArrows();

  joystickMove = getJoystickMove();

  switch (joystickMove) {
    case LEFT:
      if(currName[letterPos] != 'a') {
        currName[letterPos]--;
      }
      break;
    case RIGHT:
      if(currName[letterPos] != 'z') {
        currName[letterPos]++;
      }
      break;
    case UP:
      letterPos = min(letterPos + 1, nameSize - 1);
      break;
    case DOWN:
      letterPos = max(letterPos - 1, 0);
      break;
    default:
      break;
  }

  canScrollUp = !(currName[letterPos] == 'a');
  canScrollDown = !(currName[letterPos] == 'z');

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    saveName();

    letterPos = 0;

    if (currState == ENTER_NAME_FOR_HIGHSCORE) {
      // update highscore top with name that has been now set
      updateHighscores();

      // reinitialize current score and transition to highscore top
      currScore = 0;
      setNextState(HIGHSCORE);
    }
    else
    {
      // otherwise, the name setting option has been selected from the settings submenu we will return to
      currState = SETTINGS;
    }
  }
}

// [SOUND] : set sound settings option manager 
void setSound() {
  static const char soundText[][16] = {
    {'O', 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'O', 'F', 'F', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
  };

  scrollThrough(soundText, soundOptions, soundIndex, SETTINGS, 0, soundOptions - 1, 0, 0, false);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    EEPROM.put(soundAddress, soundOptions - soundIndex - 1);

    setSoundFromEEPROM();
    setNextState(SETTINGS);
  }
}

// [DIFFICULTY] : set difficulty settings option manager 
void setDifficulty() {
  scrollThrough(difficulty, diffOptions, diffIndex, SETTINGS, 0, diffOptions - 1, 0, 0, false);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    EEPROM.put(currDiffAddress, diffIndex);
    setNextState(SETTINGS);
  }
}

// [BRIGHTNESS] : set brightness settings option manager : lets the user navigate through the brightness submenu and press to select which brightness to change
void setBrightness() {
  static const char brightnessMatrix[][16] = {
    {'B', 'r', 'i', 'g', 'h', 't', 'n', 'e', 's', 's', ' ', ' ', ' ', ' ', ' '},
    {'M', 'a', 't', 'r', 'i', 'x', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'L', 'c', 'd', ' ', 'D', 'i', 's', 'p', 'l', 'a', 'y', ' ', ' ', ' ', ' '},
  };

  scrollThrough(brightnessMatrix, brightnessOptions, brightnessOptionIndex, SETTINGS, 1, brightnessOptions - 1, 1, 1, true);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    parseBrightnessOption(brightnessOptionIndex);
  }
} 

// [BRIGHTNESS OPTION]: parses the user's choice of component regarding brightness change and enters next corresponding state
void parseBrightnessOption(const int brightnessOptionIndex) {
  lcd.clear();

  switch(brightnessOptionIndex) {
    case CHANGE_BRIGHTNESS_MATRIX_OPTION_INDEX:
      setNextState(SET_MATRIX_BRIGHTNESS);
      break;
    case CHANGE_BRIGHTNESS_LCD_OPTION_INDEX:
      setNextState(SET_LCD_BRIGHTNESS);
      break;
    default:
      break;

  }

  // go back to settings (automatic save)
  joystickMove = getJoystickMove();
	
  if (joystickMove == DOWN) {
    setNextState(SETTINGS);
  }
}

// [MATRIX BRIGHTNESS] : TO DO COMM
void setMatrixBrightness() {
  displayMatrixBrightness();

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
    lcd.clear();
    matrixBrightness = max(matrixBrightness - 1, 0);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  }  
  else if (joystickMove == RIGHT) {
    lcd.clear();
    matrixBrightness = min(matrixBrightness + 1, 15);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  }

  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  lc.setIntensity(0, matrixBrightness);
  
  if (joystickMove == DOWN) {
    setNextState(SET_BRIGHTNESS);
  }

  canScrollUp = !(matrixBrightness == 0);
  canScrollDown = !(matrixBrightness == 15);
}

void displayMatrixBrightness() {
  lcd.setCursor(0,0);
  lcd.print("Brightness:");
  lcd.setCursor(0,1);
  lcd.print(matrixBrightness);
  displayArrows();
}

// [LCD BRIGHTNESS] : TO DO COMM
void setLcdBrightness() {
  EEPROM.get(lcdBrightnessAddress, lcdBrightness);

  displayLcdBrightness();

  analogWrite(lcdBacklightPin, lcdBrightness * LCD_BRIGHTNESS_FACTOR);

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
    lcd.clear();
    lcdBrightness = max(lcdBrightness - 1, 0);
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  }  
  else if (joystickMove == RIGHT) {
    lcd.clear();
    lcdBrightness = min(lcdBrightness + 1, maxLcdBrightness);
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  }
  else if (joystickMove == DOWN) {
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
    setNextState(SET_BRIGHTNESS);
  }

  canScrollUp = !(lcdBrightness == maxLcdBrightness);
  canScrollDown = !(lcdBrightness == 0);
}

void displayLcdBrightness() {
  lcd.setCursor(0,0);
  lcd.print("Brightness:");
  lcd.setCursor(0,1);
  lcd.print(lcdBrightness);
  displayArrows();
}