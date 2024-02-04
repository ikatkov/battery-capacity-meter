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

const uint8_t SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G          // E
};

OneButton chargeDischargeButton = OneButton(CHARGE_DISCHARGE_BUTTON_PIN, true, true);
OneButton oneAmpButton = OneButton(ONE_AMP_BUTTON_PIN, true, true);
TM1637Display display(CLK_PIN, DIO_PIN);

bool chargeState = false; // true = charge, false = discharge
bool oneAmpState = false; // false = 0.5A, true = 1A

void setChargeState(bool state);

void onChargeDischargeButtonClick()
{
    Serial.println(F("onChargeDischargeButtonClick"));
    chargeState = !chargeState;
    Serial.println(chargeState ? F("charge") : F("discharge"));
    setChargeState(chargeState);
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

void setChargeState(bool state)
{
    if (state)
    {
        // charge
        digitalWrite(CHARGE_EN_PIN, HIGH);
        pinMode(DISCHARGE_EN_PIN, OUTPUT);
        digitalWrite(DISCHARGE_EN_PIN, LOW);
    }
    else
    {
        // discharge
        digitalWrite(CHARGE_EN_PIN, LOW);
        pinMode(DISCHARGE_EN_PIN, INPUT);
    }
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
    setChargeState(false);

    Serial.begin(38400);
    Serial.println(F("booting up"));

    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);

    Serial.println(F("Display test"));
    display.setBrightness(255, true);
    display.setSegments(SEG_DONE);

    digitalWrite(BUZZER_PIN, LOW);

    // while (!Serial)
    // {
    //     delay(1);
    // }

    chargeDischargeButton.attachClick(onChargeDischargeButtonClick);
    oneAmpButton.attachClick(onOneAmpButtonClick);
}

// the loop function runs over and over again forever
long lastTimeSensorRead = millis();

void loop()
{
    chargeDischargeButton.tick();
    oneAmpButton.tick();

    if (lastTimeSensorRead + 1000 < millis())
    {
        int sensorValue = analogRead(CHARGE_I_SENSE_PIN);
        // V = (sensorValue + 0.5) * V_REF / 1024.0
        float voltage = (sensorValue + 0.5) * 2.5 / 1024.0;
        Serial.print(sensorValue);
        Serial.print("\t");
        Serial.print(voltage);
        Serial.print("\t");
        // I = 1.06437919 * V + 0.01570146
        float chargeCurrent = 1.06437919 * voltage + 0.01570146;
        if (!oneAmpState)
        {
            chargeCurrent /= 2;
        }
        Serial.println(chargeCurrent);
        lastTimeSensorRead = millis();
    }

    delay(10);
}