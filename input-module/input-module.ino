#include <Keypad.h>

char KEYS[4][4] = {{'1', '2', '3', 'A'},
                   {'4', '5', '6', 'B'},
                   {'7', '8', '9', 'C'},
                   {'*', '0', '#', 'D'}};

byte ROW_PINS[] = {11, 10, 9, 8};
byte COL_PINS[] = {7, 6, 5, 4};

Keypad keypad(makeKeymap(KEYS), ROW_PINS, COL_PINS, 4, 4);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    char key = keypad.getKey();
    if (key && keypad.getState() == KeyState::PRESSED)
    {
        Serial.flush();
        Serial.println(key);
    }
}
