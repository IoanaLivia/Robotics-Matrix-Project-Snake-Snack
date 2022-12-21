// Matrix Menu & Demo for Game

#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>
#include "Constants.h"

// control of the matrix's leds and lcd display
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// universal variable used to temporarly store different addresses
int address;

// when the current game starts
unsigned long gameStartTime = 0;

// in-game variables
int lifes = 3,
    currScore = -1,
    noOfMinutes = 0,
    noOfSeconds = 0;

// index of the current option for and total number of options for menus and submenus or game settings (difficulty, brightness, etc)
int diffIndex = 0, 
    aboutIndex = 1,
    settingsIndex = 1,
    howToIndex = 1,
    soundIndex = 1,
    menuIndex = 1,
    letterIndex = 0,
    letterPos = 0,
    brightnessOptionIndex = 1;

int currState = WELCOME;

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

// snake move and snake size
int move = UP,
    snakeSize = 3;

// arrays used to store snake components positions
int snakeRow[matrixSize * matrixSize];
int snakeCol[matrixSize * matrixSize];

// true if the snake changed size after eating
bool changedSize = false;

// time related variables related to the last time the snake moved, food / bomb was generated 
unsigned long lastMoveTime,
              generatedFood,
              generatedBomb;

// states of switch and buzzer
byte swState = LOW,
     lastSwState = LOW,
     buzzerState = HIGH;

int switchPress = NONE,
    joystickMove = NONE,
    lastJoystickMove = NONE;

bool passedShortDelay = false,
     canScrollDown = true, 
     canScrollUp = false,
     joyMoved = false;

// last time a message has been displayed on lcd, last debounce time for switch press and last blink time for food
unsigned long lastMessageTime = -1,
              lastDebounceTime = 0,
              lastBlinkFoodTime = 0,
              lastBlinkBombTime = 0;

// variables for current position of food and its state : collected / not collected
int foodCol = 0,
    foodRow = 0,
    bombCol = 0,
    bombRow = 0;

bool collectedFood = false,
     explodedBomb = false;

// joystick movement x axis and y axis initial, current and change values
int initialXValue = 510,
    initialYValue = 510,
    xValue = 0,
    yValue = 0,
    xChange = 0,
    yChange = 0;

// minimum and maximum thresholds for joystick movement coordinates
int minThreshold = 250,
    maxThreshold = 750;

// current position of the player
int currRow = 0,
    currCol = 3;

int matrixBrightness = 2;

char currName[nameSize],
     highscoreNames[highscores][nameSize];

int highscoreValues[highscores];

// highscore variables for placement achieved and iteration
int placeHighscore = -1,
    highscoreIndex = highscores - 1,
    lcdBrightness = 10;

// true if the game is in progress
bool inLevel = true;

// initialize with true to reinitialize EEPROM
bool initEEPROM = false;

unsigned long moveInterval = 20;

struct levelConfiguration {
  const int scoreThreshold,
            levelSpeed,
            spanBombsInterval,
            foodValue,
            foodSpanInterval;
  byte spanBombs;
};

// the time the function was called (for endGame function and displayFirstEndScreen function)
unsigned long calledFirstEndScreen = -1,
              calledEndGame = -1;

levelConfiguration levelsConfiguration[] = {{3, 700, 0, 1, 10000, 0},
                                            {15, 350, 0, 5, 10000, 0},
                                            {45, 550, 10000, 10, 5000, 1},
                                            {100, 300, 10000, 25, 5000, 1}};

void setup() {
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
  
  // random seed in order for each game experience to be different
  randomSeed(analogRead(0));

  // initialize led matrix
  lc.shutdown(0, false);
  lc.clearDisplay(0);

  lcd.begin(16, 2);

  // create custom chars
  lcd.createChar(CHAR_DOWN_ARROW, charDownArrow);
  lcd.createChar(CHAR_UP_ARROW, charUpArrow);
  lcd.createChar(CHAR_HEART, charHeart);
  lcd.createChar(CHAR_CLOCK, charClock);
  lcd.createChar(CHAR_STAR, charStar);
  lcd.createChar(CHAR_HUMAN, charHuman);
  //lcd.createChar(CHAR_QUESTION, charQuestion);

  currState = MENU;

  // initializing EEPROM memory
  if (initEEPROM) {
    initializeEEPROM();
  }
  else {
    setupEEPROM();
  }
}
void initializeEEPROM() {
  // current name
    resetName();

    // highscores
    resetHighscores();

    // difficulty
    EEPROM.put(currDiffAddress, 0);

    // sound
    EEPROM.put(soundAddress, HIGH);

    // led brightness
    EEPROM.put(lcdBrightnessAddress, maxLcdBrightness);
    analogWrite(lcdBacklightPin, maxLcdBrightness);

    // matrix brightness
    matrixBrightness = 2;
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
    lc.setIntensity(0, matrixBrightness);

}
void setupEEPROM() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    address += sizeof(char);
  }

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

  // difficulty
  EEPROM.get(currDiffAddress, diffIndex);
  
  // sound
  EEPROM.get(soundAddress, buzzerState);

  // led brightness
  EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  analogWrite(lcdBacklightPin, lcdBrightness * LCD_BRIGHTNESS_FACTOR);

  // matrix brightness
  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  lc.setIntensity(0, matrixBrightness);
}

void loop() {
 parseCurrState();
}

void resetName() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.put(address, char('?'));
    address += sizeof(char);
  }

  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    address += sizeof(char);
  }
}

void parseCurrState() {
  switch (currState) {
    case WELCOME:
      enterWelcome();
      break;
    case MENU:
      enterMenu();
      break;
    case START_GAME:
      enterGame();
      break;
    case SETTINGS:
      enterSettings();
      break;
    case ABOUT:
      enterAbout();
      break;
    case HOW_TO:
      enterHowTo();
      break;
    case HIGHSCORE:
      enterHighscore();
      break;
    case END_GAME:
      endGame();
      break;
    case SET_SOUND:
      setSound();
      break;
    case SET_DIFFICULTY:
      setDifficulty();
      break;
    case SET_BRIGHTNESS:
      setBrightness();
      break;
    case ENTER_NAME:
      enterName();
      break;
    case SET_MATRIX_BRIGHTNESS:
      setMatrixBrightness();
      break;
    case SET_LCD_BRIGHTNESS:
      setLcdBrightness();
      break;
    case RESET_HIGHSCORES:
      resetHighscores();
      break;
    case ENTER_NAME_FOR_HIGHSCORE:
      enterName();
      break;
    case DISPLAY_HIGHSCORE_MESSAGE:
      displayFirstEndScreen();
      break;
    case RESET_NAME:
      resetNameSetting();
    default:
      break;
  }
}
void resetNameSetting() {
  lcd.setCursor(0,0);
  lcd.print("New name: ???");
  lcd.setCursor(0,1);
  lcd.print("Press to save.");

  if(getSwitchPress() != NONE) {
    resetName();
    currState = SETTINGS;
  }

  if (getJoystickMove() == DOWN) {
    currState = SETTINGS;
  }
}

void enterWelcome() {
  static const char beginMatrix[][16] = {
    {'O', 'n', 'e', ' ', 'o', 'f', ' ', 't', 'h', 'e', ' ', 'b', 'e', 's', 't', ' '},
    {'g', 'a', 'm', 'e', 's', ' ', 'i', 's', ' ', 'b', 'a', 'c', 'k', '!', ' ', ' '},
    {'I', ' ', 'h', 'o', 'p', 'e', ' ', 'y', 'o', 'u', ' ', 'a', 'r', 'e', ' ', ' '},
    {'r', 'e', 'a', 'd', 'y', ' ', 'f', 'o', 'r', ' ', 'i', 't', ' ', '.', '.', '.'},
    {'W', 'e', 'l', 'c', 'o', 'm', 'e', ' ', 't', 'o',  ' ', 'S', 'n', 'a', 'k', 'e'},
    {'S', 'n', 'a', 'c', 'k', 's', ' ', ' ', '&', ' ', 'B', 'o', 'm', 'b', 's', '!'},
  };

  displayImage(welcomeImage);
  displayText(beginMatrix, 6);
}

void resetHighscores() {
  // reinitialize values at corresponding memory addresses
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

  // reinitialize highscoreNames and highscoreValues
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

  // go back to settings
  joystickMove = getJoystickMove();
	
  if (joystickMove == DOWN) {
    lcd.clear();
    currState = SETTINGS;
  }

}

void assignStateToBuzzer(int buzzerTone, byte buzzerState) {
  if (buzzerState == HIGH) {
    tone(buzzerPin, buzzerTone);
  } 
  else {
    noTone(buzzerPin);
  }
}

void setBrightness() {
  static const char brightnessMatrix[][16] = {
    {'B', 'r', 'i', 'g', 'h', 't', 'n', 'e', 's', 's', ' ', ' ', ' ', ' ', ' '},
    {'M', 'a', 't', 'r', 'i', 'x', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'L', 'c', 'd', ' ', 'D', 'i', 's', 'p', 'l', 'a', 'y', ' ', ' ', ' ', ' '},
  };

  scrollThrough(brightnessMatrix, brightnessOptions, brightnessOptionIndex, SETTINGS, 1, brightnessOptions - 1, 1);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    parseBrightnessOption(brightnessOptionIndex);
  }
} 

void parseBrightnessOption(const int brightnessOptionIndex) {
  lcd.clear();

  switch(brightnessOptionIndex) {
    case 1:
      currState = SET_MATRIX_BRIGHTNESS;
      break;
    case 2:
      lcd.setCursor(0,0);
      currState = SET_LCD_BRIGHTNESS;
      break;
    default:
      break;

  }

  joystickMove = getJoystickMove();

  if (joystickMove == DOWN) {
    lcd.clear();
    currState = SETTINGS;
  }
}

void enterMenu() {
  static const char menuMatrix[][16] = {
    {' ', ' ', ' ', ' ', ' ', 'M', 'e', 'n', 'u', ' ', ' ', ' ', ' ', ' ', ' '},
    {'S', 't', 'a', 'r', 't', ' ', 'G', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'H', 'i', 'g', 'h', 's', 'c', 'o', 'r', 'e', 's', ' ', ' ', ' ', ' ', ' '},
    {'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'A', 'b', 'o', 'u', 't', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'H', 'o', 'w', ' ', 't', 'o', ' ', 'p', 'l', 'a', 'y', ' ', ' ', ' ', ' '},
  };

  displayImage(welcomeImage);

  scrollThrough(menuMatrix, menuOptions, menuIndex, MENU, 1, menuOptions - 1, 1);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    parseMenuOption(menuIndex);
  }
}

void enterGame() { 
  inGameMovement();
  generateFood();
  if (levelsConfiguration[diffIndex].spanBombs == 1) {
    generateBombs();
  }
  
  displayLcdGame();  

  if (lifes == 0) {
    beep(100);
    inLevel = false;
    lcd.clear();
    lcd.setCursor(0,0);
    currState = END_GAME;
  }

  if (currScore >= levelsConfiguration[diffIndex].scoreThreshold) {
    if (diffIndex == diffOptions - 1) {
      lcd.clear();
      lcd.print(diffIndex);
      currState = END_GAME;
    }
    else {
      diffIndex++;
      beep(1000);
    }
  }
}

void enterSettings() {
  static const char settingsMatrix[][16] = {
    {' ', ' ', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' '},
    {'E', 'n', 't', 'e', 'r', ' ', 'N', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'R', 'e', 's', 'e', 't', ' ', 'N', 'a', 'm', 'e', ' ', ' ', ' ', ' ', ' '},
    {'B', 'r', 'i', 'g', 'h', 't', 'n', 'e', 's', 's', ' ', ' ', ' ', ' ', ' '},
    {'S', 'o', 'u', 'n', 'd', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'D', 'i', 'f', 'f', 'i', 'c', 'u', 'l', 't', 'y', ' ', ' ', ' ', ' ', ' '},
    {'R', 'e', 's', 'e', 't', ' ', 'T', 'o', 'p', ' ', '5', ' ', ' ', ' ', ' '},
  };

  displayImage(settingsImage);
  scrollThrough(settingsMatrix, settingsOptions, settingsIndex, MENU, 1, settingsOptions - 1, 1);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    lcd.clear();
    parseSettingsOption(settingsIndex);
  }
}


void setSound() {
  static const char soundMatrix[][16] = {
    {'S', 'o', 'u', 'n', 'd', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' '},
    {'O', 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'O', 'F', 'F', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
  };

  scrollThrough(soundMatrix, soundOptions, soundIndex, MENU, 1, soundOptions - 1, 1);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    if (soundIndex == 1) {
      EEPROM.put(soundAddress, 1);
    }
    else
    {
      EEPROM.put(soundAddress, 0);
    }

    EEPROM.get(soundAddress, buzzerState);
  }

}

void parseSettingsOption(const int settingsIndex) {
  lcd.clear();

  if (settingsIndex == 1) {
    if (currName[0] == '?')
    {
      initializeName();
    }

    // lcd.clear();
    // lcd.setCursor(0,0);
    // letterPos = 0;
    currState = ENTER_NAME;
  }
  else if (settingsIndex == 2) {
    lcd.clear();
    lcd.setCursor(0,0);
    currState = RESET_NAME;
  }
  else if (settingsIndex == 3) {
    lcd.setCursor(0,0);
    currState = SET_BRIGHTNESS;
  }
  else if (settingsIndex == 4) {
    currState = SET_SOUND;
  }
  else if (settingsIndex == 5) {
    currState = SET_DIFFICULTY;
  }
  else if (settingsIndex == 6) {
    lcd.setCursor(0,0);
    lcd.print("Top 5 highscores");
    lcd.setCursor(0,1);
    lcd.print("are now reseted!");

    currState = RESET_HIGHSCORES;
  }
}

void initializeName() {
  for(int i = 0; i < nameSize; ++i) {
        currName[i] = 'a';
  }

  saveName();

  lcd.clear();
  lcd.setCursor(0,0);
  letterPos = 0;
}

void enterAbout() {
  static const char aboutMatrix[][16] = {
    {' ', ' ', ' ', ' ', ' ', 'A', 'b', 'o', 'u', 't', ' ', ' ', ' ', ' ', ' '},
    {'S', 'n', 'a', 'k', 'e', ' ', 'S', 'n', 'a', 'c', 'k', ' ', ' ', ' ', ' '},
    {'A', 'u', 't', 'h', 'o', 'r', ' ', 'I', 'o', 'a', 'n', 'a', ' ', ' ', ' '},
    {'L', 'i', 'v', 'i', 'a', ' ', 'P', 'o', 'p', 'e', 's', 'c', 'u', ' ', ' '},
    {'G', 'i', 't', 'h', 'u', 'b', ' ', 'U', 's', 'e', 'r', 'n', 'a', 'm', 'e'},
    {'I', 'o', 'a', 'n', 'a', 'L', 'i', 'v', 'i', 'a', ' ', ' ', ' ', ' ', ' '},
  };

  displayImage(aboutImage);
  scrollThrough(aboutMatrix, aboutOptions, aboutIndex, MENU, 1, aboutOptions - 1, 1);
}

void scrollThrough(const char matrix[][16], const int options, int &index, const int returnToState, const int lowerBoundIndex, const int upperBoundIndex, const int upperBoundCursorRow) {
  if (upperBoundCursorRow == 0) {
    lcd.setCursor(0,0);
    lcd.print(matrix[index]);
    lcd.setCursor(0,1);
    lcd.print("Press to choose");
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print(matrix[0]);
    lcd.setCursor(0,1);
    lcd.print(matrix[index]);
  }

  displayArrows();

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
    lcd.clear();
    beep(600);
    index = max (index - 1, lowerBoundIndex);
  }  
  else if (joystickMove == RIGHT) {
    lcd.clear();
    beep(600);
    index  = min (index + 1, upperBoundIndex);
  }
  else if (joystickMove == DOWN) {
    lcd.clear();
    index = lowerBoundIndex;
    currState = returnToState;
  }

  canScrollUp = !(index == lowerBoundIndex);
  canScrollDown = !(index == upperBoundIndex);
}

void beep(const int toneValue) {
  if (buzzerState) {
    tone(buzzerPin, toneValue, 300);
  }
}
void enterHowTo() {
  static const char howToMatrix[][16] = {
    {' ', ' ', 'H', 'o', 'w', ' ', 't', 'o', ' ', 'p', 'l', 'a', 'y', ' ', ' '},
    {'C', 'o', 'l', 'l', 'e', 'c', 't', ' ', 'f', 'o', 'o', 'd', '!', ' ', ' '},
    {'U', 's', 'e', ' ', 'j', 'o', 'y', 's', 't', 'i', 'c', 'k', ' ', 't', 'o'},
    {'m', 'o', 'v', 'e', ' ', 'W', 'A', 'S', 'D', '.', ' ', ' ', ' ', ' ', ' '},
    {'A', 'v', 'o', 'i', 'd', ' ', 'w', 'a', 'l', 'l', 's', ' ', 'a', 'n', 'd'},
    {'b', 'o', 'm', 'b', 's', ' ', 'w', 'h', 'i', 'c', 'h', ' ', 'b', 'l', 'i'},
    {'n', 'k', ' ', 'f', 'a', 's', 't', 'e', 'r', '!', ' ', ' ', ' ', ' ', ' '},
    {'H', 'a', 'v', 'e', ' ', 'S', 's', 'o', 'm', 'e', ' ', 'F', 'u', 'n', '!'},
  };

  displayImage(howToImage);
  scrollThrough(howToMatrix, howToOptions, howToIndex, MENU, 1, howToOptions - 1, 1);
}

void enterHighscore() {
  displayImage(highscoreImage);

  lcd.setCursor(0,0);
  lcd.print("Highscores");
  lcd.setCursor(0,1);

  for (int j = 0; j < nameSize; ++j) {
    lcd.print(highscoreNames[highscoreIndex][j]);
  }

  lcd.print(" ");
  //lcd.print(highscoreValues[highscoreIndex]);
  displayNumber(highscoreValues[highscoreIndex], 1, 5);
  displayArrows();

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
     highscoreIndex = min(highscoreIndex + 1, highscores - 1);
  }  
  else if (joystickMove == RIGHT) {
    highscoreIndex = max(highscoreIndex - 1, 0);
  }
  else if (joystickMove == DOWN) {
    lcd.clear();
    highscoreIndex = highscores - 1;
    currState = MENU;
  }

  canScrollUp = !(highscoreIndex == highscores - 1);
  canScrollDown = !(highscoreIndex == 0);
}

void endGame() {
  //static long called = -1;

  if (calledEndGame == -1) { //called
    displayImage(endGameImage);
    //called = millis();
    calledEndGame = millis();
  }

  if (millis() - calledEndGame > 3000 && calledEndGame != -1) { //if (millis() - called > 3000 && called != -1) {
    // if (calledFirstEndScreen != -1) {
    //   lcd.clear();
    //   lcd.setCursor(0,0);
    calledFirstEndScreen == -1;
    //}
    
    displaySecondEndScreen();

    int press = getSwitchPress();
    if (press != NONE) {
      lcd.clear();
      lc.setLed(0, currRow, currCol, 0);
      lc.setLed(0, foodRow , foodCol, 0);
      currCol = 0;
      currRow = 0;
      calledFirstEndScreen == -1;
      gameStartTime = 0;
      lifes = 3;
      noOfMinutes = 0;
      noOfSeconds = 0;
      foodCol = 0;
      foodRow = 0;

      if (placeHighscore != -1) {
        if(currName[0] == '?') {
          initializeName();
          currState = ENTER_NAME_FOR_HIGHSCORE;
        }
        else {
          currScore = 0;
          lcd.clear();
          lcd.setCursor(0,0);
          currState = HIGHSCORE;
        }
      }
      else
      {
        currScore = 0;
        currState = MENU;
      }
    }
  }
  else
  { 
    displayFirstEndScreen();
  }
}

// based on pressing history, returns corresponding type of press or NONE if absent
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

// returns current joystick move 
int getJoystickMove() {
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

void displayFirstEndScreen() {
  if (calledFirstEndScreen == -1) {
    calledFirstEndScreen = millis();
  }

  address = currNameStartingAddress;

  lcd.setCursor(0,0);
  lcd.print("Congrats!");
  lcd.setCursor(13,0);

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    lcd.print(currName[i]);
    address += sizeof(char);
  }

  lcd.setCursor(0,1);
  lcd.print("Level: ");
  displayNumber(diffIndex, 1, 9);
  lcd.print("!");
}

void updateHighscores() {
    for (int i = highscores - 1; i >= 0; i--) {
      if (currScore > highscoreValues[i] && currScore != 0) {
        placeHighscore = i;
        break;
      }
    }

    if (placeHighscore != -1 && currName[0] != '?') {
      for (int i = 0; i < placeHighscore; ++i) {
        for (int j = 0; j < nameSize; j++) {
          highscoreNames[i][j] = highscoreNames[i + 1][j];
        }
        highscoreValues[i] =  highscoreValues[i + 1];
      }

      highscoreValues[placeHighscore] = currScore;

      for (int j = 0; j < nameSize; j++) {
        highscoreNames[placeHighscore][j] = currName[j];
      }

      saveHighscores();
    }
}
void displaySecondEndScreen() {
  lcd.setCursor(0,0);
  lcd.print("Your score      ");
  displayNumber(currScore, 0, 13);

  if (placeHighscore == -1) {
    updateHighscores();
  }

  if (placeHighscore != -1) {
    address = currNameStartingAddress;

    for (int i = 0 ; i < nameSize; i++) {
        EEPROM.get(address, currName[i]);
        lcd.print(currName[i]);
        address += sizeof(char);
    }
    
    beep(800);
    lcd.setCursor(0,1);
    displayImage(highscoreImage);
    lcd.print("is in Top 5 !!!");
  }
  else
  { 
    beep(200);
    lcd.setCursor(0,1);
    displayImage(sadImage);
    lcd.print("...");
    lcd.setCursor(0,1);
    lcd.print("Not a highscore...");
  }
}

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

// display arrows based on the current values of canScrollUp and canScrollDown
void displayArrows() {
  // up arrow
  lcd.setCursor(15,0);
  if (canScrollUp) {
    lcd.write(byte(1));
  }
  else
  {
    lcd.print(" ");
  }

  //down arrow
  lcd.setCursor(15,1);
  if (canScrollDown) {
    lcd.write(byte(0));
  }
  else
  {
    lcd.print(" ");
  }
}

// displays all lcd game required information plus timer
void displayLcdGame() {
  static long lastScoreChange = 0;
  static int changeInterval = 1000;

  displayLife();
  displayScore();
  displayTimer();
  displayName();
  displayDifficulty();
}

void displayScore() {
  lcd.setCursor(4,0);
  lcd.write(CHAR_STAR);
  lcd.setCursor(6,0);
  if (currScore < 10) {
    lcd.print(digits[currScore]);
  }
  else {
    lcd.print(digits[currScore / 10]);
    lcd.print(digits[currScore % 10]);
  }
}

void displayLife() {
  lcd.setCursor(0,0);
  lcd.write(CHAR_HEART);
  lcd.setCursor(2,0);
  lcd.print(digits[lifes]);
}

void displayTimer() {
  lcd.setCursor(10,0);
  lcd.write(CHAR_CLOCK);
  lcd.setCursor(11,0);

  noOfMinutes = (millis() - gameStartTime) / 60000;
  if (noOfMinutes < 10) {
    lcd.print("0");
    lcd.setCursor(12,0);
  }

  lcd.print(noOfMinutes);
  lcd.print(":");
  lcd.setCursor(14,0);

  noOfSeconds = (millis() - gameStartTime) / 1000 -  60 * noOfMinutes;

  if (noOfSeconds >= 60) {
    noOfMinutes++;
    noOfSeconds -= 60;
  }

  if (noOfSeconds < 10) {
    lcd.print("0");
  }
	
  lcd.print(noOfSeconds);
}

void displayName() {
  lcd.setCursor(0,1);
  lcd.write(CHAR_HUMAN);
  lcd.print(" ");

  address = currNameStartingAddress;
  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.get(address, currName[i]);
    lcd.print(currName[i]);
    address += sizeof(char);
  }
}

void displayDifficulty() {
  lcd.setCursor(8,1); 
  lcd.print("  "); 
  lcd.setCursor(10,1);
  lcd.print(difficulty[diffIndex]);
}

//universal function to display text letter by letter at scrollInterval on two rows (used for welcome text)
void displayText(const char matrix[][16], const int noOfLines) {
  static int scrollInterval = 175;
  static int lineIndex = 0;
  static int columnIndex = 0;
  static int cursorRow = 0;

  if (lastMessageTime == -1 && millis() != 0) {
    lcd.setCursor(0, cursorRow);
    lastMessageTime = millis();
  }

  if (millis() - lastMessageTime >= scrollInterval && columnIndex < 17) {
    lcd.print(matrix[lineIndex][columnIndex]);
    lastMessageTime = millis();
    columnIndex += 1;
  }
  
  if (columnIndex == 17) {
    lastMessageTime = -1;
    columnIndex = 0;
    lineIndex += 1;
    if (lineIndex == noOfLines) {
      lcd.clear();
      currState = MENU;
    }
    if (cursorRow == 1) {
      lcd.clear();
      cursorRow = 0;
    }
    else {
      cursorRow = 1;
    }
  }
}

void generateFood() {
  static bool expiredFoodTime = false;
  static int intervalMultiplier = 5000;
  static unsigned long foodOnScreenInterval = 0;

  // depending on difficulty, the foodOnScreenInterval is either loger or shorter
  foodOnScreenInterval = levelsConfiguration[diffIndex].foodSpanInterval;

  if (inLevel) {
    if (collectedFood || currScore == -1 || expiredFoodTime) {
      collectedFood = false;
      lastBlinkFoodTime = millis();
      if (currScore == -1) currScore = 0;
      generatedFood = millis();
      expiredFoodTime = false;

      // avoid the case when the food spawns on the current position of the player or of a bomb, could lead to confusion / increased difficulty
      bool badFood = true;
      while (badFood) {
        badFood = false;
        foodCol = rand() % 8, foodRow = rand() % 8;
        for (int i=0; i < snakeSize; i++) {
          if (foodCol == snakeCol[i] && foodRow == snakeRow[i]) {
            badFood = true;
          }
        }
        if(bombsBitmap[foodRow][foodCol] == 1) {
          badFood = true;
        }
      }
    }

    if (millis() - generatedFood >= foodOnScreenInterval) {
      expiredFoodTime = true;
      lc.setLed(0, foodCol, foodRow, 0);
    }

    blinkFood(foodCol, foodRow, 100);
  }
}


void generateBombs() {
  static int intervalMultiplier = 7000;
  static bool generateNewBomb = false;
  static unsigned long generateBombsInterval = 0;

  // depending on difficulty, the foodOnScreenInterval is either loger or shorter
  generateBombsInterval = levelsConfiguration[diffIndex].spanBombsInterval;

  if (inLevel) {
    if (generateNewBomb) {
      generatedBomb = millis();
      generateNewBomb = false;

      // avoid the case when the bomb spawns on the current position of the player or of the food, could lead to confusion
      bool badBomb = true;
      while (badBomb) {
        badBomb = false;
        bombCol = rand() % 8, bombRow = rand() % 8;
        for (int i = 0; i < snakeSize; i++) {
          if (bombCol == snakeCol[i] && bombRow == snakeRow[i]) {
            badBomb = true;
          }
        }
        if(bombCol == foodCol && bombRow == foodRow){
          badBomb = true;
        }
        if(bombsBitmap[bombRow][bombCol] == 1) {
          badBomb = true;
        }
      }

      bombsBitmap[bombRow][bombCol] = 1;
    }

    
    if (millis() - generatedBomb >= generateBombsInterval) {
      generateNewBomb = true;
    }

    blinkBombs(700);
  }
  
}

void inGameMovement() {
  static const int intervalMultiplier = 200;

  int joystickMove = getJoystickMove();

  if (joystickMove != NONE) {
    if ((move == UP || move == DOWN) && (joystickMove == LEFT || joystickMove == RIGHT)) {
      move = joystickMove;
    }
    else if ((joystickMove == UP || joystickMove == DOWN) && (move == LEFT || move == RIGHT)) {
      move = joystickMove;
    }
  }

  if (millis() - lastMoveTime > levelsConfiguration[diffIndex].levelSpeed) { 
    lastMoveTime = millis();

    switch(move){
      case LEFT:
        if (currCol == 0) {
          lifes = 0;
        }
        currCol = max(currCol - 1, 0);
        break;
      case RIGHT:
        if (currCol == matrixSize - 1) {
          lifes = 0;
        }
        currCol = min(currCol + 1, matrixSize - 1);
        break;
      case UP:
        if (currRow == matrixSize - 1) {
          lifes = 0;
        }
        currRow = min(currRow + 1, matrixSize - 1);
        break;
      case DOWN:
        if (currRow == 0) {
          lifes = 0;
        }
        currRow = max(currRow - 1, 0);
        break;
    }

    if (!changedSize) {
      lc.setLed(0, snakeCol[snakeSize-1], snakeRow[snakeSize-1], 0);
    }
    changedSize = false;

    for (int i=snakeSize-1; i>0; i--) {
      snakeRow[i] = snakeRow[i-1];
      snakeCol[i] = snakeCol[i-1];
    }

    snakeRow[0] = currRow;
    snakeCol[0] = currCol;
    lc.setLed(0, currCol, currRow, 1);

    for (int i=1; i<snakeSize; i++){
      if (snakeRow[i] == currRow && snakeCol[i] == currCol) {
        lifes = 0;
      }
    }

    
    if (currCol == foodCol && currRow == foodRow) {
      beep(600);
      collectedFood = true;
      currScore += levelsConfiguration[diffIndex].foodValue;
      snakeSize += 1;
      changedSize = true;
    }


    if(bombsBitmap[currRow][currCol]) {
      beep(200);
      bombsBitmap[currRow][currCol] = 0;
      lifes--;
    }
    
  }
}


void parseMenuOption(const int menuIndex) {
    lcd.clear();

    for (int row = 0; row < matrixSize; row++) {
      for (int col = 0; col < matrixSize; col++) {
        lc.setLed(0, row, col, 0);
      }
    }

    switch(menuIndex) {
      case 1:
        currState = START_GAME;
        gameStartTime = millis();
        lastJoystickMove = NONE;
        placeHighscore = -1;
        calledEndGame = -1;
        inLevel = true;
        snakeSize = 3;
        for (int i=0; i<snakeSize; i++) {
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
        move = UP;
        changedSize = false;
        collectedFood = false;
        lastMoveTime = millis();
        generatedFood = millis();

        for (int i = 0; i < matrixSize; ++i) {
          for (int j = 0; j < matrixSize; ++j) {
            bombsBitmap[i][j] = 0;
          }
        }

        break;
      case 2:
        readHighscores();
        address = highscoreStartingAddress;
        lcd.clear();
        lcd.setCursor(0,0);
        currState = HIGHSCORE;
        enterHighscore();
        break;
      case 3:
        currState = SETTINGS;
        enterSettings();
        break;
      case 4:
        currState = ABOUT;
        enterAbout();
        break;
      case 5:
        currState = HOW_TO;
        enterHowTo();
        break;
      case 6:
        currState = RESET_HIGHSCORES;
        break;
      default:
        break;
    }
}

void blinkFood (const int col, const int row, const int blinkInterval) {
  static byte blinkState = 1;

  if (millis() - lastBlinkFoodTime >= blinkInterval) {
    blinkState = !blinkState;
    lc.setLed(0, col, row, blinkState);
    lastBlinkFoodTime = millis();
  }
}

void blinkBombs (const int blinkInterval) {
  static byte blinkState = 1;

  for (int i=0; i<matrixSize; i++) {
    for (int j=0; j<matrixSize; j++) {
      if (bombsBitmap[i][j]) {
        lc.setLed(0, j, i, blinkState);
      }
    }
  }

  if (millis() - lastBlinkBombTime >= blinkInterval) {
    blinkState = !blinkState;
    lastBlinkBombTime = millis();
  }
}

void assignStateToBuzzer(int buzzerTone, byte buzzerState, int buzzerPin) {
  if (buzzerState == HIGH) {
    tone(buzzerPin, buzzerTone);
  } 
  else {
    noTone(buzzerPin);
  }
}

void setDifficulty() {
  scrollThrough(difficulty, diffOptions, diffIndex, SETTINGS, 0, diffOptions - 1, 0);

  switchPress = getSwitchPress();

  if (switchPress != NONE) {
    EEPROM.put(currDiffAddress, diffIndex);
  }
}

void setMatrixBrightness() {
  lcd.setCursor(0,0);
  lcd.print("Brightness:");
  lcd.setCursor(0,1);
  lcd.print(matrixBrightness);
  displayArrows();

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
    lcd.clear();
    currState = SET_BRIGHTNESS;
  }

  canScrollUp = !(matrixBrightness == 0);
  canScrollDown = !(matrixBrightness == 15);
}

void setLcdBrightness() {
  EEPROM.get(lcdBrightnessAddress, lcdBrightness);

  lcd.setCursor(0,0);
  lcd.print("Brightness:");
  lcd.setCursor(0,1);
  lcd.print(lcdBrightness);
  displayArrows();

  analogWrite(lcdBacklightPin, lcdBrightness * LCD_BRIGHTNESS_FACTOR);

  joystickMove = getJoystickMove();

  if (joystickMove == LEFT) {
    lcd.clear();
    lcdBrightness = min(lcdBrightness + 1, maxLcdBrightness);
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  }  
  else if (joystickMove == RIGHT) {
    lcd.clear();
    lcdBrightness = max(lcdBrightness - 1, 0);
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  }
  else if (joystickMove == DOWN) {
    lcd.clear();
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
    currState = SET_BRIGHTNESS;
  }

  canScrollUp = !(lcdBrightness == maxLcdBrightness);
  canScrollDown = !(lcdBrightness == 0);
}

void enterName() {
  Serial.println(currName);

  lcd.setCursor(0, 1);
  lcd.print("Press to save.");
  lcd.setCursor(0, 0);

  for (int i = 0; i < nameSize; ++i) {
    lcd.print(currName[i]);
    Serial.println(currName[i]);
  }

  lcd.setCursor(9,0); 
  lcd.print("Max:3");
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
    if (currState == ENTER_NAME_FOR_HIGHSCORE) {
      lcd.clear();
      lcd.setCursor(0,0);
      updateHighscores();
      currScore = 0;
      currState = HIGHSCORE;
    }
    else
    {
      currState = SETTINGS;
    }
  }
}

void saveName() {
  address = currNameStartingAddress;

  for (int i = 0 ; i < nameSize; i++) {
    EEPROM.put(address, currName[i]);
    address += sizeof(char);
  }

}

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


void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}