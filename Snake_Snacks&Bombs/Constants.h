// Header for game constants

#ifndef CONSTANTS_H
#define CONSTANTS_H

// universal none value
#define NONE -1

// states
#define WELCOME 0
#define MENU 1
#define SETTINGS 2
#define START_GAME 3
#define HIGHSCORE 4
#define ABOUT 5
#define HOW_TO 6
#define END_GAME 7
#define ENTER_NAME 8
#define SET_BRIGHTNESS 9
#define SET_SOUND 10
#define SET_DIFFICULTY 11
#define SET_MATRIX_BRIGHTNESS 12
#define SET_LCD_BRIGHTNESS 13
#define RESET_HIGHSCORES 14
#define ENTER_NAME_FOR_HIGHSCORE 15
#define DISPLAY_HIGHSCORE_MESSAGE 16
#define RESET_NAME 17

// difficulties indexes
#define EASY 0
#define MEDIUM 1
#define HARD 2
#define INSANE 3

// custom chars
#define CHAR_DOWN_ARROW 0
#define CHAR_UP_ARROW 1
#define CHAR_HEART 2
#define CHAR_CLOCK 3
#define CHAR_STAR 4
#define CHAR_HUMAN 5
#define CHAR_QUESTION 6

// joystick movement directions
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// switch presses
#define LONG_PRESS 1
#define SHORT_PRESS 0

// multipling factors to reduce options for brightness
#define LCD_BRIGHTNESS_FACTOR 20
#define MATRIX_BRIGHTNESS_FACTOR

// pins
const int pinSW = 2,
          pinY = A0,
          pinX = A1;

const byte dinPin = 12,
      clockPin = 11,
      loadPin = 10;

const byte rs = 9,
           en = 8,
           d4 = 7,
           d5 = 13,
           d6 = 5,
           d7 = 4;

const byte lcdBacklightPin = 6,
           buzzerPin = 3;


// memory addresses for EEPROM
const int matrixBrightnessAddress = 0,
          soundAddress = 2,
          currNameStartingAddress = 20,
          currDiffAddress = 4,
          highscoreStartingAddress = 30,
          lcdBrightnessAddress = 100;

// customized chars
const byte charDownArrow[8] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b01110,
	0b00100
};

const byte charUpArrow[8] = {
	0b00100,
	0b01110,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

const byte charHeart[8] = {
	0b00000,
	0b01010,
	0b11111,
	0b11111,
	0b01110,
	0b00100,
	0b00000,
	0b00000
};

const byte charClock[8] = {
	0b00000,
	0b01110,
	0b10011,
	0b10101,
	0b10001,
	0b01110,
	0b00000,
	0b00000
};

const byte charStar[8] = {
  B00000,
  B00100,
  B10101,
  B01110,
  B01110,
  B10101,
  B00100,
  B00000
};

const byte charHuman[8] = {
  B00000,
  B01110,
  B01110,
  B01110,
  B00100,
  B01110,
  B00100,
  B01010
};

const byte charQuestion[8] = {
  B11111,
  B00001,
  B00001,
  B00001,
  B00010,
  B00100,
  B00000,
  B00100
};

// custom char array to display digits on the lcd
const char digits[10] = {'0','1','2','3','4','5','6','7','8','9'};

const byte matrixSize = 8;

const int maxLcdBrightness = 10;

const int blinkInterval = 300,
          debounceDelayLong = 3000,
          debounceDelay = 100;

const char difficulty[][16] = {
  {'E','A','S','Y',' ',' '},
  {'M','E','D','I','U','M'},
  {'H','A','R','D',' ',' '},
  {'I','N','S','A','N','E'},
};

const int highscores = 5,
          nameSize = 3;

// initialize snake with corresponding values for size 3
const int snakeStartRow[] = {2, 1, 0};
const int snakeStartCol[] = {3, 3, 3};


// matrix images 
const uint64_t welcomeImage = 0xffbda1bd85ad81ff;
const uint64_t highscoreImage = 0x7e1818183c7e7e7e;
const uint64_t startGameImage = 0x00040c1c3c1c0c04;
const uint64_t settingsImage = 0x3c3c203c3c0c3c3c;
const uint64_t aboutImage = 0x2424243c24242418;
const uint64_t howToImage = 0x2424243c3c242424;
const uint64_t sadImage = 0x0000243c00240000;
const uint64_t endGameImage = 0x007e7e7e7e7e7e00;

// number of possible variations of choice for certain setting
const int brightnessOptions = 3,
          aboutOptions = 6,
          diffOptions = 4,
          settingsOptions = 7,
          howToOptions = 8,
          soundOptions = 3,
          menuOptions = 6;


#endif