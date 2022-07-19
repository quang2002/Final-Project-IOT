#include <LiquidCrystal.h>
#include "smart-lock.h"

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

SmartLock smart_lock(lcd);

void setup()
{
    Serial.begin(115200);
    smart_lock.setup();
    smart_lock.home();
}

void loop()
{
    char key = Serial.read();

    if (key != -1)
        smart_lock.process(key);

    smart_lock.render();

}
