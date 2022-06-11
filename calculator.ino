#include <Keypad.h>
#include <LiquidCrystal.h>

// 4x4 keypad
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'C', '0', '=', '/'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

// LCD
LiquidCrystal lcd_1(14, 15, 16, 17, 18, 19);
int pos = 0;

// Variables to process the calculation
char str[18]; // to store inputted key
int nStr = 0; // number of inputted key
boolean done = false; // check if a calculation is complete
boolean error = false; // check if a calculation is error

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  lcd_1.begin(16, 2);
  // Make the str empty
  for (int i = 0; i < 16; i++)
    str[i] = '\0';
  Serial.begin(9600);
}

void loop() {
  double result; // result of the calculation
  long result2; // if the result is an integer, parse the result to long
  String res; // result in string type to print
  char customKey = customKeypad.getKey();

  if (customKey) {
    // if the previous calculation is complete, clear the LCD
    // reset all variables
    if (done) {
      pos = 0;
      for (int i = 0; i < 16; i++)
        str[i] = '\0';
      nStr = 0;
      pos = 0;
      lcd_1.clear();
      done = false;
    }
    // Receive key and print on LCD
    if (customKey != '=' && customKey != 'C' && nStr < 16) {
      lcd_1.setCursor(pos, 0);
      lcd_1.print(customKey);
      pos++;
      str[nStr] = customKey;
      nStr++;
    } else
      // if user press '=', start doing calculation
      if (customKey == '=') {
        str[nStr] = customKey;
        nStr++;
        // get the result
        result = eval(String(str), &done, &error);
        // check if result is an integer, parse it to long type to print
        if (result == (long)result) {
          res = String(result, 0);
          lcd_1.setCursor(16 - res.length(), 1);
          if (error == false)
            lcd_1.print(res);
          // else print it as a double
        } else {
          res = String(result, 3);
          lcd_1.setCursor(16 - res.length(), 1);
          if (error == false)
            lcd_1.print(res);
        }
        done = true;

        // Delete inputted key
      } else if (customKey == 'C') {
        if (nStr > 0) {
          // Delete last character of string
          str[nStr] = '\0';
          nStr--;
          // Delete in LCD
          pos--;
          lcd_1.setCursor(pos, 0);
          lcd_1.print(" ");
        }
      }
  }
}
// this function receive a string of the calculation,
// 2 pointers to done and error to change their value
// and return the result
double eval(String str, boolean *done, boolean *error) {
  double num[16]; // to store the number
  char op[9]; // to store the operation
  char tmp[18]; // to get the char sequence of the number to parse
  double tmpNum; // parsed value of tmp[]
  int nnum = 0, nop = 0, nTmp = 0; // array size of num[], op[] and tmp[]
  *error = false; // first error will be false

  // set tmp to empty string
  for (int i = 0; i < 18; i++)
    tmp[i] = '\0';
  // Split numbers and operators out of the string
  for (int i = 0; i < str.length(); i++) {
    if (str.charAt(i) >= 48 && str.charAt(i) <= 57) {
      tmp[nTmp] += str[i];
      nTmp++;
    }
    else {
      // get number
      tmpNum = 0;
      for (int j = 0; j < strlen(tmp); j++)
        tmpNum = tmpNum * 10 + (tmp[j] - 48);
      if (str[i - 1] >= 48) {
        num[nnum] = tmpNum;
        nnum++;
      }
      for (int j = 0; j < 18; j++)
        tmp[j] = '\0';
      nTmp = 0;
      // get operator
      if (str[i] != '=') {
        op[nop] = str[i];
        nop++;
      }
    }
  }

  int oppos = -1;
  int numpos = 0;
  // Check if there are any negative integers
  for (int i = 0; i < str.length(); i++) {
    if (str.charAt(i) == 43 || str.charAt(i) == 45 || str.charAt(i) == 42 || str.charAt(i) == 47) {
      oppos++;
      if (str.charAt(i - 1) >= 48 && str.charAt(i - 1) <= 57)
        numpos++;
    }
    if (str.charAt(i) == '-' && !(str.charAt(i - 1) >= 48 && str.charAt(i - 1) <= 57)) {
      num[numpos] = -num[numpos];
      for (int j = oppos; j < nop - 1; j++)
        op[j] = op[j + 1];
      nop--;
      oppos--;
    }
  }

  // Check error
  if (nnum - 1 != nop) {
    lcd_1.setCursor(0, 1);
    lcd_1.print("Systax error");
    *error = true;
    return 0;
  }

  // Execute * / operation
  for (int i = 0; i < nop; i++) {
    if (op[i] == '*' || op[i] == '/') {
      if (op[i] == '*')
        num[i] = num[i] * num[i + 1];
      else {
        if (num[i + 1] == 0) {
          lcd_1.setCursor(0, 1);
          lcd_1.print("error dividing 0");
          *error = true;
          return 0;
        }
        num[i] = num[i] / num[i + 1];
      }
      for (int j = i + 1; j < nnum - 1; j++)
        num[j] = num[j + 1];
      nnum--;
      for (int j = i; j < nop - 1; j++)
        op[j] = op[j + 1];
      nop--;
      i--;
    }
  }

  // Execute + - operation
  for (int i = 0; i < nop; i++) {
    if (op[i] == '+' || op[i] == '-') {
      if (op[i] == '+')
        num[i] = num[i] + num[i + 1];
      else
        num[i] = num[i] - num[i + 1];
      for (int j = i + 1; j < nnum - 1; j++)
        num[j] = num[j + 1];
      nnum--;
      for (int j = i; j < nop - 1; j++)
        op[j] = op[j + 1];
      nop--;
      i--;
    }
  }

  // Check if the result is out of range
  if (num[0] > 67108864 || num[0] < -67108864) { // -2^26 -> 2^26
    lcd_1.setCursor(0, 1);
    lcd_1.print("Out of range");
    *error = true;
  } else
    return num[0];
}
