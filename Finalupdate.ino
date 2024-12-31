#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define IR_SENSOR_PIN 2 
const int SERVO_PIN = 3;
const int RED_LED = 12;
const int GREEN_LED = 11;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int rowPins[4] = {10, 9, 8, 7}; 
int colPins[3] = {6, 5, 4};     

char keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

class KeypadController {
private:
    int *rowPins, *colPins;

public:
    String input = "";
    bool is_input = 0;
    bool is_confirm = 0;

    KeypadController(int rowPins[], int colPins[]) {
        this->rowPins = rowPins;
        this->colPins = colPins;
    }

    void setup() {
        for (int i = 0; i < 4; i++)
            pinMode(rowPins[i], OUTPUT);
        for (int i = 0; i < 3; i++)
            pinMode(colPins[i], INPUT_PULLUP);
    }

    char getKeypadKey() {
        for (int row = 0; row < 4; row++) {
            digitalWrite(rowPins[row], LOW);
            for (int col = 0; col < 3; col++) {
                if (digitalRead(colPins[col]) == LOW) {
                    while (digitalRead(colPins[col]) == LOW);
                    digitalWrite(rowPins[row], HIGH);
                    return keys[row][col];
                }
            }
            digitalWrite(rowPins[row], HIGH);
        }
        return '\0';
    }

    void loop() {
        char K = this->getKeypadKey();
        if (K == '\0')
            return;

        if (K == '*') {
            this->is_input = 1;
            return;
        }
        if (K == '#') {
            if(this->is_confirm != 1){
                this->clear();
                return;
            } else {
                if(this->input != ""){
                    this->clear();
                    return;
                } else {
                    this->clear();
                }
            }
        }
        this->input += K;
    }

    void clear() {
        this->input = "";
        is_input = 0;
    }
};

class ServoController {
private:
    int servoPin;

public:
    Servo servo;
    int servoValue = 0;

    ServoController(int servoPin) {
        this->servoPin = servoPin;
    }

    void setup() {
        this->servo.attach(this->servoPin);
        this->servo.write(this->servoValue);
    }

    void moveLeft(int step = 3) {
        this->servoValue = 180;
        this->servo.write(this->servoValue);
    }

    void moveRight(int step = 3) {
        this->servoValue = 0;
        this->servo.write(this->servoValue);
    }
};

KeypadController keypad(rowPins, colPins);
ServoController door(SERVO_PIN);

String PASSWORD = "", CUR_PASSWORD = "";

void lcd_print(String text, String text2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text);
    lcd.setCursor(0, 1);
    lcd.print(text2);
}
void setPassword() {
    point1:
        keypad.is_confirm = 0;
        lcd_print("Set Password", "");
        String tempPassword = ""; 
        CUR_PASSWORD = ""; 

        // Step 1: Input the initial password
        while (true) {
            keypad.loop();
            if (keypad.input != CUR_PASSWORD) {
                lcd_print("Set Password", keypad.input);
                Serial.println("Inputting password: " + keypad.input); 
            }
            CUR_PASSWORD = keypad.input;

            if (keypad.is_input) {
                if (keypad.input != "") {
                    if (keypad.input.length() < 4) {
                     
                        lcd_print("Minimum 4 Digits", "");
                        Serial.println("Password Is Small");
                        delay(5000);
                         keypad.clear();
                        lcd_print("Set Password", "");
                    } else {
                        tempPassword = keypad.input;
                        Serial.println("Temp password captured: " + tempPassword); 
                        keypad.clear();
                        digitalWrite(GREEN_LED, LOW);
                        break; 
                    }
                } else {
                    lcd_print("Empty!", "");
                    Serial.println("Password Is Empty");
                    delay(5000);
                     keypad.clear();
                    lcd_print("Set Password", "");
                }
                
            }
            delay(20);
        }

        lcd_print("Confirm Password", "");
        CUR_PASSWORD = ""; 
        keypad.is_confirm = 1; 

        // Step 3: Confirm the password
        while (true) {
            keypad.loop();

            if (keypad.input == "#") {
                keypad.clear();
                goto point1;
            }

            if (keypad.input != CUR_PASSWORD) {
                lcd_print("Confirm Password", keypad.input); 
                Serial.println("Confirming password: " + keypad.input); 
            }
            CUR_PASSWORD = keypad.input;

            if (keypad.is_input) {
                if (keypad.input != "") {
                    // Check if confirmation matches
                    if (keypad.input == tempPassword) { 
                        PASSWORD = tempPassword;
                        Serial.println("Password confirmed: " + PASSWORD); 
                        keypad.clear();
                        lcd_print("Password Set!", "");
                        delay(2000);
                        door.moveRight(3); 
                        return;
                    } else {
                        lcd_print("Mismatch", "Try Again.");
                        Serial.println("Password mismatch");
                        delay(5000);
                        lcd_print("Confirm Password", "");
                    }
                    keypad.clear();
                } else {
                    lcd_print("Empty!", "");
                    Serial.println("Password Is Empty");
                    delay(5000);
                    keypad.clear();
                    lcd_print("Confirm Password", "");
                }
            }            
            delay(20);
        }
        keypad.is_confirm = 0;
}



void lockDoor() {
    setPassword();
    point2:
        lcd_print(" Enter Password ", "");
        while (true) {
            keypad.loop();

            if (keypad.input == "#"){
                keypad.clear();
                goto point2;
            }

            if (keypad.input != CUR_PASSWORD) {
                lcd_print(" Enter Password ", keypad.input);
            }
            CUR_PASSWORD = keypad.input;
            if (keypad.is_input) {
                if (keypad.input != "") {
                    if (keypad.input == PASSWORD) {
                        keypad.clear();
                        lcd_print("Correct ", "Take Your Phone");
                        door.moveLeft(3);
                        delay(13000);
                        lcd_print("Put Your Phone ", "");
                        digitalWrite(RED_LED, HIGH);
                        return;
                    } else {
                        lcd_print(" Wrong Password ", "");
                    }
                    delay(2000);
                    keypad.clear();
                    lcd_print(" Enter Password ", "");
                } else {
                    lcd_print("Empty!", "");
                    Serial.println("Password Is Empty");
                    delay(5000);
                    keypad.clear();
                    lcd_print("Enter Password", "");
                }
            }
            delay(20);
        }
}

void setup() {
    Serial.begin(9600);
    keypad.setup();
    door.setup();
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);

    digitalWrite(RED_LED, HIGH);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Put Your Phone ");
    Serial.println("System initialized");
    door.moveLeft();
}

void loop() {
    int sensorValue = digitalRead(IR_SENSOR_PIN);
    Serial.print("IR Sensor Value: ");
    Serial.println(sensorValue);
//Adjust based on sensor behavior
    if (sensorValue == LOW) { 
        Serial.println("Object detected!");
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        delay(2000);
        door.moveLeft(3);
        lockDoor();
    } else {
        Serial.println("No object detected.");
    }

    delay(500); 
}