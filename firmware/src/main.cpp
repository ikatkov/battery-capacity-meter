// TODO: handle battery disconnect mid cycle
// TODO: wrong battery polarity

#include <Arduino.h>
#include <OneButton.h>
#include <TM1637Display.h>

#define BATTERY_VOLTAGE_PIN A0
#define CHARGE_I_SENSE_PIN A1
#define BATTERY_POLARITY_SENSE_PIN A2

#define ONE_A_MODE_PIN A3
#define CHARGE_EN_PIN 12
#define NOT_ONE_A_MODE_PIN 11
#define DISCHARGE_EN_PIN 10
#define BUZZER_PIN 9

// display
#define DIO_PIN 8
#define CLK_PIN 7

#define CHARGE_DISCHARGE_BUTTON_PIN 3
#define ONE_AMP_BUTTON_PIN 2

//  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
const uint8_t TEXT_ON[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G, 0, 0                    // n
};

const uint8_t TEXT_CHRG[] = {
    SEG_A | SEG_D | SEG_E | SEG_F,         // C
    SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // H
    SEG_E | SEG_G,                         // r
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F  // G
};

const uint8_t TEXT_TEST[] = {
    SEG_D | SEG_E | SEG_F | SEG_G,         // t
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G, // S
    SEG_D | SEG_E | SEG_F | SEG_G          // t
};

const uint8_t TEXT_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, // d
    SEG_C | SEG_D | SEG_E | SEG_G,         // o
    SEG_C | SEG_E | SEG_G,                 // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G  // E
};

const uint8_t TEXT_BATT[] = {
    SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         // B
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_D | SEG_E | SEG_F | SEG_G,                 // t
    SEG_D | SEG_E | SEG_F | SEG_G                  // t
};

const uint8_t TEXT_LO[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, // L
    SEG_C | SEG_D | SEG_E | SEG_G, 0, 0    // O
};

const uint8_t TEXT_PERCENT[] = {
    0, 0,
    SEG_A | SEG_B | SEG_F | SEG_G, // %
    SEG_C | SEG_D | SEG_E | SEG_G  //%
};

OneButton chargeDischargeButton = OneButton(CHARGE_DISCHARGE_BUTTON_PIN, true, true);
OneButton oneAmpButton = OneButton(ONE_AMP_BUTTON_PIN, true, true);
TM1637Display display(CLK_PIN, DIO_PIN);

#define BAT_LOW_VOLTAGE_CUT_OFF 3000
#define BAT_HIGH_VOLTAGE_CUT_OFF 4200

enum STATE
{
    IDLE,
    CHARGING,
    DISCHARGING,
    DONE
};

STATE state = IDLE;       // true = charge, false = discharge
bool oneAmpState = false; // false = 0.5A, true = 1A

int chargeCurrent = 0;
int batteryVoltage = 0;
int batteryPercentage = 0;
int batteryCapacitymAh = 0;

unsigned long lastTimeChargeCurrentRead = millis();
unsigned long lastTimeBatteryVoltageRead = millis();
unsigned long lastTimeMessageDisplayed = millis();
unsigned long lastTimeBatteryPolarityRead = millis();
unsigned long dischargeIntervalStartedMs = 0;
bool messageToggle = false;

void readBatteryVoltage(int delayMs = 0);

void setState(STATE stateValue)
{
    state = stateValue;
    if (state == CHARGING)
    {
        display.setSegments(TEXT_CHRG);
        // charge
        digitalWrite(CHARGE_EN_PIN, HIGH);
        pinMode(DISCHARGE_EN_PIN, OUTPUT);
        digitalWrite(DISCHARGE_EN_PIN, LOW);
    }
    else if (state == DISCHARGING)
    {
        batteryCapacitymAh = 0;
        display.setSegments(TEXT_TEST);
        // discharge
        digitalWrite(CHARGE_EN_PIN, LOW);
        pinMode(DISCHARGE_EN_PIN, INPUT);
    }
    else
    {
        digitalWrite(CHARGE_EN_PIN, LOW);
        pinMode(DISCHARGE_EN_PIN, OUTPUT);
        digitalWrite(DISCHARGE_EN_PIN, LOW);
    }
}

void beep(int duratuionMs = 100)
{
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
}

void longBeep()
{
    beep(300);
}

void trippleBeep()
{
    beep();
    delay(200);
    beep();
    delay(200);
    beep();
    delay(200);
}

void onChargeDischargeButtonClick()
{
    Serial.println(F("onChargeDischargeButtonClick"));
    beep();
    if (state == IDLE)
    {
        // check if battery connected
        readBatteryVoltage();
        if (batteryVoltage < 1000)
        {
            Serial.println(F("Battery disconnected"));
            display.setSegments(TEXT_BATT);
            longBeep();
            longBeep();
            longBeep();
            setState(IDLE);
        }
        else
        {
            Serial.println(F("Start charging..."));
            setState(CHARGING);
            display.setSegments(TEXT_CHRG);
            delay(1000);
        }
    }
    else if (state == CHARGING)
    {
        // before starting the discharge, check if battery voltage is too low
        readBatteryVoltage();
        if (batteryVoltage <= BAT_LOW_VOLTAGE_CUT_OFF)
        {
            Serial.println(F("Battery voltage too low"));
            display.setSegments(TEXT_BATT);
            longBeep();
            delay(1000);
            display.setSegments(TEXT_LO);
            longBeep();
            delay(1000);
            display.setSegments(TEXT_CHRG);
            return;
        }
        else
        {
            Serial.println(F("Starting discharge"));
            display.setSegments(TEXT_TEST);
            trippleBeep();

            setState(DISCHARGING);
            dischargeIntervalStartedMs = millis();
            Serial.print(F("Discharge interval started:\t"));
            Serial.println(dischargeIntervalStartedMs);
        }
    }
    else if (state == DISCHARGING)
    {
        Serial.println(F("Stop discharge"));
        setState(DONE);
    }
    else if (state == DONE)
    {
        Serial.println(F("Go to idle"));
        setState(IDLE);
    }
}

void onOneAmpButtonClick()
{
    Serial.println(F("onOneAmpButtonClick"));
    oneAmpState = !oneAmpState;
    Serial.println(oneAmpState ? F(" 1A") : F(" 0.5A"));
    if (oneAmpState)
    {
        // 1A
        digitalWrite(ONE_A_MODE_PIN, HIGH);
        digitalWrite(NOT_ONE_A_MODE_PIN, LOW);
    }
    else
    {
        // 0.5A
        digitalWrite(ONE_A_MODE_PIN, LOW);
        digitalWrite(NOT_ONE_A_MODE_PIN, HIGH);
    }
    // reset battery capacity
    batteryCapacitymAh = 0;
}

void setOneAmpMode(bool state)
{
    oneAmpState = state;
    if (oneAmpState)
    {
        // 1A
        digitalWrite(ONE_A_MODE_PIN, HIGH);
        digitalWrite(NOT_ONE_A_MODE_PIN, LOW);
    }
    else
    {
        // 0.5A
        digitalWrite(ONE_A_MODE_PIN, LOW);
        digitalWrite(NOT_ONE_A_MODE_PIN, HIGH);
    }
}

float computeChargeCurrent(float voltage)
{
    if (oneAmpState)
    {
        return -0.50940504 * pow(voltage, 4) + 1.11893064 * pow(voltage, 3) + -0.80032945 * pow(voltage, 2) + 1.19824953 * voltage + -0.01151128;
    }
    else
    {
        return 0.27035568 * pow(voltage, 4) + -0.51039031 * pow(voltage, 3) + 0.31013550 * pow(voltage, 2) + 0.44244364 * voltage + 0.00128515;
    }
}

void readBatteryVoltage(int delayMs = 0)
{
    if (state == CHARGING)
    {
        // stop charging to read battery voltage
        digitalWrite(CHARGE_EN_PIN, LOW);
        delay(delayMs);
    }

    // read battery voltage
    int sensorValue = analogRead(BATTERY_VOLTAGE_PIN);
    // V = (sensorValue + 0.5) * V_REF / 1024.0
    batteryVoltage = round(1000 * 2 * ((sensorValue + 0.5) * 2.5 / 1024.0));
    batteryPercentage = map(batteryVoltage, BAT_LOW_VOLTAGE_CUT_OFF, BAT_HIGH_VOLTAGE_CUT_OFF, 0, 100);

    if (state == CHARGING)
    {
        // restore charge state
        digitalWrite(CHARGE_EN_PIN, HIGH);
    }

    Serial.print("Voltage =\t");
    Serial.print(sensorValue);
    Serial.print("\t");
    Serial.print(batteryVoltage);
    Serial.print("\t");
    Serial.print(batteryPercentage);
    Serial.println("%");
}

void readChargeCurrent()
{
    int sensorValue = analogRead(CHARGE_I_SENSE_PIN);
    // V = (sensorValue + 0.5) * V_REF / 1024.0
    float voltage = (sensorValue + 0.5) * 2.5 / 1024.0;
    chargeCurrent = round(1000 * max(0, computeChargeCurrent(voltage))); // charging current in mA
    Serial.print("Charge Current =\t");
    Serial.print(sensorValue);
    Serial.print("\t");
    Serial.print(voltage);
    Serial.print("\t");
    Serial.println(chargeCurrent);
}

void setup()
{
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGE_I_SENSE_PIN, INPUT);
    pinMode(BATTERY_POLARITY_SENSE_PIN, INPUT);

    pinMode(CHARGE_EN_PIN, OUTPUT);
    pinMode(ONE_A_MODE_PIN, OUTPUT);
    pinMode(NOT_ONE_A_MODE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    analogReference(EXTERNAL);

    pinMode(DIO_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);

    pinMode(DISCHARGE_EN_PIN, INPUT);

    setOneAmpMode(false);
    setState(IDLE);

    Serial.begin(38400);
    Serial.println(F("booting up"));

    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);

    display.setBrightness(255, true);
    display.clear();
    display.setSegments(TEXT_ON);

    digitalWrite(BUZZER_PIN, LOW);

    chargeDischargeButton.attachClick(onChargeDischargeButtonClick);
    oneAmpButton.attachClick(onOneAmpButtonClick);
}

void loop()
{
    chargeDischargeButton.tick();
    oneAmpButton.tick();

    if (lastTimeBatteryPolarityRead + 1000 < millis())
    {
        bool correctBatteryPolarity = !digitalRead(BATTERY_POLARITY_SENSE_PIN);
        if (!correctBatteryPolarity)
        {
            Serial.println(F("Wrong battery polarity"));
            display.setSegments(TEXT_BATT);
            longBeep();
            longBeep();
            longBeep();
        }
        lastTimeBatteryPolarityRead = millis();
    }

    // ============ Charge =================
    if (state == CHARGING)
    {
        // read battery voltage every 5 seconds when charging
        if (lastTimeBatteryVoltageRead + 5000 < millis())
        {
            readBatteryVoltage(10);
            lastTimeBatteryVoltageRead = millis();
            if (batteryVoltage < 1000)
            {
                Serial.println(F("Battery disconnected"));
                display.setSegments(TEXT_BATT);
                longBeep();
                longBeep();
                longBeep();
                setState(IDLE);
            }
        }

        // display battery voltage and percentage every 1 seconds
        if (millis() - lastTimeMessageDisplayed > 1000)
        {
            lastTimeMessageDisplayed = millis();
            messageToggle = !messageToggle;
            if (messageToggle)
            {
                display.showNumberDecEx(batteryVoltage, 0b10000000, false);
            }
            else
            {
                display.setSegments(TEXT_PERCENT);
                display.showNumberDec(batteryPercentage, false, 2, 0);
            }
        }

        // read charge current every 1 seconds when charging
        if (lastTimeChargeCurrentRead + 1000 < millis())
        {
            readChargeCurrent();
            lastTimeChargeCurrentRead = millis();
        }

        // are we done charging?
        if ((oneAmpState && chargeCurrent < 90) || (!oneAmpState && chargeCurrent < 45))
        {
            // check if battery connected
            readBatteryVoltage(1000);
            if (batteryVoltage < 1000)
            {
                Serial.println(F("Battery disconnected"));
                display.setSegments(TEXT_BATT);
                longBeep();
                longBeep();
                longBeep();
                setState(IDLE);
            }
            else
            {
                Serial.println(F("Starting discharge, after full charge"));
                display.setSegments(TEXT_TEST);
                trippleBeep();

                dischargeIntervalStartedMs = millis();
                setState(DISCHARGING);
            }
        }
    }
    else if (state == DISCHARGING)
    {
        // ============ Discharge ============
        // read battery voltage every 1 seconds when discharging
        if (lastTimeBatteryVoltageRead + 1000 < millis())
        {
            readBatteryVoltage();
            lastTimeBatteryVoltageRead = millis();

            // 500mA*h discharge in mA*second is 500/3600 = 0.13888888888mAh per millisecond
            unsigned long elapsedTimeMs = millis() - dischargeIntervalStartedMs;
            Serial.print(F("elapsedTimeMs=\t"));
            Serial.println(elapsedTimeMs);
            double batteryCapacity = elapsedTimeMs * (oneAmpState ? 1000 : 500) / 3600 / 1000;
            batteryCapacitymAh = round(batteryCapacity);

            Serial.print(F("Discharge battery capacity (mA*h)=\t"));
            Serial.println(batteryCapacitymAh);
            display.showNumberDec(batteryCapacitymAh);
        }

        if (batteryVoltage < BAT_LOW_VOLTAGE_CUT_OFF)
        {
            Serial.println(F("Done discharge, battery voltage too low"));
            display.setSegments(TEXT_DONE);
            longBeep();
            // disconnect both discharge
            setState(DONE);
            lastTimeMessageDisplayed = millis();
        }
    }
    else if (state == DONE)
    {
        if (millis() - lastTimeMessageDisplayed > 1000)
        {
            lastTimeMessageDisplayed = millis();
            messageToggle = !messageToggle;
            if (messageToggle)
            {
                display.setSegments(TEXT_DONE);
            }
            else
            {
                display.showNumberDec(batteryCapacitymAh);
            }
        }
    }
    else if (state == IDLE)
    {
        if (millis() - lastTimeMessageDisplayed > 1000)
        {
            lastTimeMessageDisplayed = millis();
            messageToggle = !messageToggle;
            if (messageToggle)
            {
                display.setSegments(TEXT_ON);
            }
            else
            {
                readBatteryVoltage();
                // if voltage is too low, there is no battery connected
                if (batteryVoltage > 1000)
                    display.showNumberDecEx(batteryVoltage, 0b10000000, false);
            }
        }
    }

    delay(10);
}
