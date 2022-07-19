#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Servo.h>

enum SmartLockMenu
{
    HOME,
    USER_LOGIN,
    SET_USER_PASSWORD,
    SET_ADMIN_PASSWORD,
    ADMIN_LOGIN,
    ADMIN
};

class SmartLock
{
private:
    LiquidCrystal &lcd;
    SmartLockMenu state;
    Servo servo;
    static const int piezo = (6);
    static const int sercurity_pin = (4);
    char password[7];
    char admin_password[7];

    bool is_locked;
    bool old_is_locked;
    bool is_refresh;

    int wrong_count;

public:
    SmartLock(LiquidCrystal &lcd) : lcd(lcd), is_refresh(false), is_locked(true), old_is_locked(false), wrong_count(0)
    {
    }

    void setup()
    {
        lcd.begin(16, 2);
        lcd.cursor();
        servo.attach(5, 1000, 2000);
        pinMode(this->piezo, OUTPUT);
        pinMode(this->sercurity_pin, INPUT_PULLUP);
    }

    void lost_security()
    {
        for (int i = 0; i < 50; i++)
        {
            digitalWrite(SmartLock::piezo, HIGH);
            delay(100);
            digitalWrite(SmartLock::piezo, LOW);
            delay(100);
        }
    }

    void home()
    {
        lcd.clear();
        lcd.print("   SMART LOCK   ");
        lcd.setCursor(0, 1);
        lcd.print("A-UNLOCK|B-ADMIN");
    }

    void correct_sound()
    {
        // play sound for correct password
        tone(this->piezo, 5000, 1000);
        delay(1000);
        noTone(this->piezo);
    }

    void incorrect_sound()
    {
        // play sound for incorrect password
        tone(this->piezo, 100, 1000);
        delay(1000);
        noTone(this->piezo);
    }

    void key_sound()
    {
        // play sound for key press
        tone(this->piezo, 10000, 100);
    }

    void login(const char *password)
    {
        lcd.clear();
        lcd.print("Enter password: ");
        lcd.setCursor(0, 1);
        lcd.print("     ");
        lcd.print(password);
    }

    void set_password(const char *password)
    {
        lcd.clear();
        lcd.print("  Set password: ");
        lcd.setCursor(0, 1);
        lcd.print("     ");
        lcd.print(password);
    }

    void admin()
    {
        lcd.clear();
        lcd.print("A - USER PASS");
        lcd.setCursor(0, 1);
        lcd.print("B - ADMIN PASS");
    }

    void wrong()
    {
        lcd.clear();
        lcd.print(" WRONG PASSWORD ");
        this->incorrect_sound();
        
        this->wrong_count++;
        if (this->wrong_count >= 3)
            this->lost_security();
    }

    void correct()
    {
        lcd.clear();
        lcd.print("CORRECT PASSWORD");
        this->correct_sound();
        this->wrong_count = 0;
    }

    void render()
    {
        if (this->is_refresh)
        {

            switch (state)
            {
            case SmartLockMenu::HOME:
                this->home();
                break;
            case SmartLockMenu::USER_LOGIN:
                this->login(this->password);
                break;
            case SmartLockMenu::ADMIN_LOGIN:
                this->login(this->admin_password);
                break;
            case SmartLockMenu::SET_USER_PASSWORD:
                this->set_password(this->password);
                break;
            case SmartLockMenu::SET_ADMIN_PASSWORD:
                this->set_password(this->admin_password);
                break;
            case SmartLockMenu::ADMIN:
                this->admin();
                break;
            default:
                break;
            }

            this->is_refresh = false;
        }

        if (this->is_locked != this->old_is_locked)
        {
            servo.write(this->is_locked ? 180 : 0);
            this->old_is_locked = this->is_locked;
        }

        if (this->is_locked && !digitalRead(this->sercurity_pin))
            this->lost_security();
    }

    void process(char key)
    {
        switch (this->state)
        {
        case SmartLockMenu::HOME:
        {
            if (key == 'A')
            {
                memset(this->password, 0, sizeof(this->password));
                this->state = SmartLockMenu::USER_LOGIN;
                this->is_refresh = true;
            }
            else if (key == 'B')
            {
                memset(this->admin_password, 0, sizeof(this->admin_password));
                this->state = SmartLockMenu::ADMIN_LOGIN;
                this->is_refresh = true;
            }
            break;
        }
        case SmartLockMenu::ADMIN:
        {
            if (key == 'A')
            {
                memset(this->password, 0, sizeof(this->password));
                this->state = SmartLockMenu::SET_USER_PASSWORD;
                this->is_refresh = true;
            }
            else if (key == 'B')
            {
                memset(this->admin_password, 0, sizeof(this->admin_password));
                this->state = SmartLockMenu::SET_ADMIN_PASSWORD;
                this->is_refresh = true;
            }
            break;
        }
        case SmartLockMenu::SET_USER_PASSWORD:
        {
            if ('0' <= key && key <= '9')
            {
                int index = strlen(this->password);
                if (index < 6)
                {
                    this->password[index] = key;
                }

                if (index == 5)
                {
                    EEPROM.put(0, this->password);
                    this->state = SmartLockMenu::HOME;
                }
                this->is_refresh = true;
            }
            break;
        }
        case SmartLockMenu::SET_ADMIN_PASSWORD:
        {
            if ('0' <= key && key <= '9')
            {
                int index = strlen(this->admin_password);
                if (index < 6)
                {
                    this->admin_password[index] = key;
                }

                if (index == 5)
                {
                    EEPROM.put(7, this->admin_password);
                    this->state = SmartLockMenu::HOME;
                }
                this->is_refresh = true;
            }
            break;
        }
        case SmartLockMenu::USER_LOGIN:
        {
            if ('0' <= key && key <= '9')
            {
                int index = strlen(this->password);
                if (index < 6)
                {
                    this->password[index] = key;
                }

                if (index == 5)
                {
                    char temp[7];
                    EEPROM.get(0, temp);
                    if (strcmp(temp, this->password) == 0)
                    {
                        this->is_locked = false;
                        this->correct();
                    }
                    else
                    {
                        this->is_locked = true;
                        this->wrong();
                    }
                    this->state = SmartLockMenu::HOME;
                }
                this->is_refresh = true;
            }
            break;
        }
        case SmartLockMenu::ADMIN_LOGIN:
        {
            if ('0' <= key && key <= '9')
            {
                int index = strlen(this->admin_password);
                if (index < 6)
                {
                    this->admin_password[index] = key;
                }

                if (index == 5)
                {
                    char temp[7];
                    EEPROM.get(7, temp);
                    if (strcmp(temp, this->admin_password) == 0)
                    {
                        this->correct();
                        this->state = SmartLockMenu::ADMIN;
                    }
                    else
                    {
                        this->wrong();
                        this->state = SmartLockMenu::HOME;
                    }
                }
                this->is_refresh = true;
            }
            else
                break;
        }
        }

        if (key == 'D')
        {
            this->is_locked = true;
            this->is_refresh = true;
        }
        else if (key == 'C')
        {
            this->state = SmartLockMenu::HOME;
            this->is_refresh = true;
        }

        this->key_sound();
    }
};
