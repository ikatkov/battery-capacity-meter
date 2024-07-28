#include <Arduino.h>
#include <OneButton.h>
#include <TM1637Display.h>
#include <movingAvg.h>
#include <CustomSegments.h>

#define BATTERY_VOLTAGE_PIN A0
#define CHARGE_I_SENSE_PIN A1
#define BATTERY_POLARITY_SENSE_PIN A2
#define OVERVOLTAGE_SENSE_PIN 4

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



OneButton chargeDischargeButton = OneButton(CHARGE_DISCHARGE_BUTTON_PIN, true, true);
OneButton oneAmpButton = OneButton(ONE_AMP_BUTTON_PIN, true, true);
TM1637Display display(CLK_PIN, DIO_PIN);

#define BAT_LOW_VOLTAGE_CUT_OFF 3000
#define BAT_HIGH_VOLTAGE_CUT_OFF 4150
#define BAT_HIGH_VOLTAGE_STORAGE_CUT_OFF 3800

enum CYCLE_MODE
{
    CHARGE_ONLY,
    CHANGE_DISCHARGE,
    CHANGE_DISCHARGE_STORAGE,
    STORAGE_ONLY
};

enum STATE
{
    IDLE,
    CHARGING,
    SECOND_CHARGING,
    DISCHARGING,
    DONE,
    ERROR,
};

STATE state = IDLE;
CYCLE_MODE cycleMode = CHANGE_DISCHARGE_STORAGE;
bool oneAmpState = false; // false = 0.5A, true = 1A

movingAvg chargeCurrent(12);  // use 12 data points for the moving average, every 5sec it's 60sec averaging
movingAvg batteryVoltage(12); // use 12 data points for the moving average, every 5sec it's 60sec averaging when charging, 12sec when discharging

int batteryPercentage = 0;
int batteryCapacitymAh = 0;

unsigned long lastTimeChargeCurrentRead = millis();
unsigned long lastTimeBatteryVoltageRead = millis();
unsigned long lastTimeMessageDisplayed = millis();
unsigned long lastTimeBatteryPolarityRead = millis();
unsigned long dischargeIntervalStartedMs = 0;
bool messageToggle = false;
byte messageToggleTristate = 0;

void readBatteryVoltage(int delayMs = 0);

void setCycleMode(CYCLE_MODE cycleModeValue)
{
    cycleMode = cycleModeValue;
    if (cycleMode == CHARGE_ONLY)
    {
        display.setSegments(TEXT_CHRG);
        delay(500);
        display.setSegments(TEXT_ONLY);
        delay(500);
    }
    else if (cycleMode == CHANGE_DISCHARGE)
    {
        display.setSegments(TEXT_CHRG);
        delay(500);
        display.setSegments(TEXT_TEST);
        delay(500);
    }
    else if (cycleMode == CHANGE_DISCHARGE_STORAGE)
    {
        display.setSegments(TEXT_CHRG);
        delay(500);
        display.setSegments(TEXT_TEST);
        delay(500);
        display.setSegments(TEXT_STOR);
        delay(500);
    }
    else if (cycleMode == STORAGE_ONLY)
    {
        display.setSegments(TEXT_STOR);
        delay(500);
        display.setSegments(TEXT_ONLY);
        delay(500);
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

void setState(STATE stateValue)
{
    state = stateValue;
    if (state == CHARGING || state == SECOND_CHARGING)
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
        batteryVoltage.reset();
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
    if (state == DONE)
    {
        batteryVoltage.reset();
        display.setSegments(TEXT_DONE);
        lastTimeMessageDisplayed = millis();
        longBeep();
        longBeep();
        delay(500);
        longBeep();
        longBeep();
        delay(500);
        longBeep();
        longBeep();
    }
    if (state == ERROR)
    {
        batteryVoltage.reset();
    }
}

void onChargeDischargeButtonLngPressStart()
{
    Serial.println(F("onChargeDischargeButtonLngPressStart"));
    longBeep();
    if (state == IDLE)
    {
        setCycleMode(CYCLE_MODE((cycleMode + 1) % 4));
    }
}

void onChargeDischargeButtonClick()
{
    Serial.println(F("onChargeDischargeButtonClick"));
    beep();
    if (state == IDLE)
    {
        // check if battery connected
        readBatteryVoltage();
        if (batteryVoltage.getAvg() < 1000)
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
        if (batteryVoltage.getAvg() <= BAT_LOW_VOLTAGE_CUT_OFF)
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
    else if (state == DONE || state == ERROR)
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
    if (state == DISCHARGING)
    {
        display.setSegments(TEXT_TEST);
        trippleBeep();
        // reset battery capacity
        batteryCapacitymAh = 0;
        dischargeIntervalStartedMs = millis();
        Serial.print(F("Discharge interval started:\t"));
        Serial.println(dischargeIntervalStartedMs);
    }
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

void readBatteryVoltage(int delayMs)
{
    if (state == CHARGING)
    {
        // stop charging to read battery voltage
        digitalWrite(CHARGE_EN_PIN, LOW);
        delay(delayMs);
    }

    // read battery voltage
    int sensorValue = analogRead(BATTERY_VOLTAGE_PIN);
    // voltage higher than some floating voltage
    if (sensorValue > 150)
    {
        // V = (sensorValue + 0.5) * V_REF / 1024.0
        int batteryVoltageValue = round(1000 * 2 * ((sensorValue + 0.5) * 2.5 / 1024.0));
        batteryVoltage.reading(batteryVoltageValue);
        batteryPercentage = map(batteryVoltage.getAvg(), BAT_LOW_VOLTAGE_CUT_OFF, BAT_HIGH_VOLTAGE_CUT_OFF, 0, 100);

        Serial.print("Voltage =\t");
        Serial.print(sensorValue);
        Serial.print("\t");
        Serial.print(batteryVoltageValue);
        Serial.print("\t");
        Serial.print(batteryVoltage.getAvg());
        Serial.print("\t");
        Serial.print(batteryPercentage);
        Serial.println("%");
    }
    else
    {
        batteryVoltage.reset();
    }

    if (state == CHARGING)
    {
        // restore charge state
        digitalWrite(CHARGE_EN_PIN, HIGH);
    }
}

void readChargeCurrent()
{
    int sensorValue = analogRead(CHARGE_I_SENSE_PIN);
    // V = (sensorValue + 0.5) * V_REF / 1024.0
    float voltage = (sensorValue + 0.5) * 2.5 / 1024.0;
    int chargeCurrentValue = round(1000 * max(0, computeChargeCurrent(voltage))); // charging current in mA
    chargeCurrent.reading(chargeCurrentValue);
    Serial.print("Charge Current =\t");
    Serial.print(sensorValue);
    Serial.print("\t");
    Serial.print(voltage);
    Serial.print("\t");
    Serial.print(chargeCurrentValue);
    Serial.print("\t");
    Serial.println(chargeCurrent.getAvg());
}

void setup()
{
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGE_I_SENSE_PIN, INPUT);
    pinMode(BATTERY_POLARITY_SENSE_PIN, INPUT);
    pinMode(OVERVOLTAGE_SENSE_PIN, INPUT);

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
    setCycleMode(CHANGE_DISCHARGE);

    Serial.begin(38400);
    Serial.println(F("booting up"));

    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);

    display.setBrightness(255, true);
    display.clear();

    digitalWrite(BUZZER_PIN, LOW);

    chargeDischargeButton.attachClick(onChargeDischargeButtonClick);
    chargeDischargeButton.attachLongPressStart(onChargeDischargeButtonLngPressStart);
    oneAmpButton.attachClick(onOneAmpButtonClick);

    chargeCurrent.begin();
    batteryVoltage.begin();
}

void loop()
{
    chargeDischargeButton.tick();
    oneAmpButton.tick();

    if (lastTimeBatteryPolarityRead + 500 < millis())
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

        bool overVoltage = digitalRead(OVERVOLTAGE_SENSE_PIN);
        if (overVoltage)
        {
            Serial.println(F("Overvoltage"));
            display.setSegments(TEXT_ERR_1_OVERVOLTAGE);
            longBeep();
            longBeep();
            longBeep();
        }
        lastTimeBatteryPolarityRead = millis();
    }

    // ============ Charge =================
    if (state == CHARGING || state == SECOND_CHARGING)
    {
        // read battery voltage every 5 seconds when charging
        if (lastTimeBatteryVoltageRead + 5000 < millis())
        {
            readBatteryVoltage(10);
            lastTimeBatteryVoltageRead = millis();
            if (batteryVoltage.getAvg() < 1000)
            {
                Serial.println(F("Battery disconnected"));
                display.setSegments(TEXT_ERR_2_BATT_DISCONNECTD_MID_CHARGE);
                longBeep();
                longBeep();
                longBeep();
                setState(ERROR);
                return;
            }
        }

        // display battery voltage and percentage every 1 seconds
        if (millis() - lastTimeMessageDisplayed > 1000)
        {
            lastTimeMessageDisplayed = millis();
            messageToggle = !messageToggle;
            if (messageToggle)
            {
                display.showNumberDecEx(batteryVoltage.getAvg(), 0b10000000, false);
            }
            else
            {
                display.setSegments(TEXT_PERCENT);
                display.showNumberDec(batteryPercentage, false, 2, 0);
            }
        }

        // are we done charging for storage?
        if (batteryVoltage.getAvg() > BAT_HIGH_VOLTAGE_STORAGE_CUT_OFF &&
            (cycleMode == STORAGE_ONLY || (cycleMode == CHANGE_DISCHARGE_STORAGE && state == SECOND_CHARGING)))
        {
            Serial.println(F("Done charging for storage"));
            setState(DONE);
        }
        // read charge current every 1 seconds when charging
        if (lastTimeChargeCurrentRead + 1000 < millis())
        {
            readChargeCurrent();
            lastTimeChargeCurrentRead = millis();
        }

        if (chargeCurrent.getAvg() < 50 || batteryVoltage.getAvg() > BAT_HIGH_VOLTAGE_CUT_OFF)
        {
            // check if battery connected
            readBatteryVoltage(1000);
            if (batteryVoltage.getAvg() < 1000)
            {
                Serial.println(F("Battery disconnected mid charge"));
                display.setSegments(TEXT_ERR_2_BATT_DISCONNECTD_MID_CHARGE);
                longBeep();
                longBeep();
                longBeep();
                setState(ERROR);
                return;
            }
            // charging is done
            if (cycleMode == CHARGE_ONLY)
            {
                Serial.println(F("Done charging only"));
                setState(DONE);
            }
            else if (cycleMode == CHANGE_DISCHARGE || (cycleMode == CHANGE_DISCHARGE_STORAGE && state == CHARGING))
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
            double batteryCapacity = (double)elapsedTimeMs / 3600 / 1000 * (oneAmpState ? 1000 : 500);
            batteryCapacitymAh = round(batteryCapacity);

            Serial.print(F("Discharge battery capacity (mA*h)=\t"));
            Serial.println(batteryCapacitymAh);
            display.showNumberDec(batteryCapacitymAh);

            messageToggleTristate = (messageToggleTristate + 1) % 3;
            if (messageToggleTristate == 0)
            {
                display.showNumberDec(batteryCapacitymAh);
            }
            else if (messageToggleTristate == 1)
            {
                display.setSegments(TEXT_PERCENT);
                display.showNumberDec(batteryPercentage, false, 2, 0);
            }
            else
            {
                display.showNumberDecEx(batteryVoltage.getAvg(), 0b10000000, false);
            }
        }

        if (batteryVoltage.getAvg() < 1000)
        {
            Serial.println(F("Battery disconnected mid discharge"));
            display.setSegments(TEXT_ERR_3_BATT_DISCONNECTD_MID_DISCHARGE);
            longBeep();
            longBeep();
            longBeep();
            setState(ERROR);
            return;
        }
        else if (batteryVoltage.getAvg() < BAT_LOW_VOLTAGE_CUT_OFF)
        {
            Serial.println(F("Done discharge, battery voltage too low"));
            if (cycleMode == CHANGE_DISCHARGE)
            {
                setState(DONE);
            }
            else if (cycleMode == CHANGE_DISCHARGE_STORAGE)
            {
                Serial.println(F("Done discharge, battery voltage too low. Starting charge to storage voltage..."));
                setState(SECOND_CHARGING);
            }
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
            else if (cycleMode != CHARGE_ONLY && cycleMode != STORAGE_ONLY)
            {
                Serial.println(batteryCapacitymAh);
                display.showNumberDec(batteryCapacitymAh);
            }
        }
    }
    else if (state == IDLE)
    {
        if (millis() - lastTimeMessageDisplayed > 500)
        {
            lastTimeMessageDisplayed = millis();
            messageToggle = !messageToggle;
            if (messageToggle)
            {
                if (cycleMode == CHARGE_ONLY)
                    display.setSegments(TEXT_ON1);
                else if (cycleMode == CHANGE_DISCHARGE)
                    display.setSegments(TEXT_ON2);
                else if (cycleMode == CHANGE_DISCHARGE_STORAGE)
                    display.setSegments(TEXT_ON3);
                else if (cycleMode == STORAGE_ONLY)
                    display.setSegments(TEXT_ON4);
            }
            else
            {
                readBatteryVoltage();
                // if voltage is too low, there is no battery connected
                if (batteryVoltage.getAvg() > 1000)
                    display.showNumberDecEx(batteryVoltage.getAvg(), 0b10000000, false);
            }
        }
    }

    delay(10);
}
