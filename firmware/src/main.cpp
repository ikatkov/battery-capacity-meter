#include <Arduino.h>
#include <OneButton.h>

#define CHARGE_CURRENT_PIN A7
#define BATTERY_VOLTAGE_PIN A0

#define ONE_AMP_PIN 2 // OUTPUT LOW = 1A, OUTPUT HIGH = INVALID, INPUT 0.5A
#define CHARGE_DISCHARGE_PIN 3

#define CHARGE_DISCHARGE_BUTTON_PIN 6
#define ONE_AMP_BUTTON_PIN 7
#define ONE_AMP_LED_PIN 8

// #define BUZZER_PIN 4

OneButton chargeDischargeButton = OneButton(CHARGE_DISCHARGE_BUTTON_PIN, true, true);
OneButton oneAmpButton = OneButton(ONE_AMP_BUTTON_PIN, true, true);

bool chargeDischargeState = true; // true = charge, false = discharge
bool oneAmpState = false;         // false = 0.5A, true = 1A

void onChargeDischargeButtonClick()
{
    Serial.println(F("onChargeDischargeButtonClick"));
    chargeDischargeState = !chargeDischargeState;
    digitalWrite(CHARGE_DISCHARGE_PIN, chargeDischargeState);
}

void onOneAmpButtonClick()
{
    Serial.println(F("onOneAmpButtonClick"));
    oneAmpState = !oneAmpState;
    Serial.println(oneAmpState ? F(" 1A") : F(" 0.5A"));
    if (oneAmpState)
    {
        // 1A
        pinMode(ONE_AMP_PIN, OUTPUT);
        digitalWrite(ONE_AMP_PIN, LOW);
        digitalWrite(ONE_AMP_LED_PIN, HIGH);
    }
    else
    {
        // 0.5A
        pinMode(ONE_AMP_PIN, INPUT);
        digitalWrite(ONE_AMP_LED_PIN, LOW);
        digitalWrite(ONE_AMP_LED_PIN, LOW);
    }
}

void setup()
{
    pinMode(CHARGE_DISCHARGE_PIN, OUTPUT);
    digitalWrite(CHARGE_DISCHARGE_PIN, HIGH); // HIGH = charge, LOW = discharge

    pinMode(ONE_AMP_LED_PIN, OUTPUT);
    pinMode(ONE_AMP_PIN, INPUT);
    digitalWrite(ONE_AMP_LED_PIN, LOW);
    digitalWrite(ONE_AMP_LED_PIN, LOW);

    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGE_CURRENT_PIN, INPUT);
    analogReference(EXTERNAL);

    Serial.begin(38400);
    // while (!Serial)
    // {
    //     delay(1);
    // }

    Serial.println(F("booting up"));
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
        int sensorValue = analogRead(CHARGE_CURRENT_PIN);
        // V = (sensorValue + 0.5) * V_REF / 1024.0
        float voltage = (sensorValue + 0.5) * 2.5 / 1024.0;
        Serial.print(sensorValue);
        Serial.print("\t");
        Serial.print(voltage);
        Serial.print("\t");
        // I = 1.06437919 * V + 0.01570146
        float chargeCurrent = 1.06437919 * voltage + 0.01570146;
        if (oneAmpState)
        {
            chargeCurrent *= 2;
        }
        Serial.println(chargeCurrent);
        lastTimeSensorRead = millis();
    }

    delay(10);
}