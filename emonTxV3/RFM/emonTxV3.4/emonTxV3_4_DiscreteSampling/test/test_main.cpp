// http://docs.platformio.org/en/feature-platformio-30/platforms/unit_testing.html#example

// Basic unit test to test opperation of ATmega328 by flashing LED (pin 9) and checking output

#include <Arduino.h>
#include <unity.h>

#ifdef UNIT_TEST

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }


void test_6_pin_number(void) {
    TEST_ASSERT_EQUAL(6, 6);
}

void test_led_state_high(void) {
    digitalWrite(6, HIGH);
    TEST_ASSERT_EQUAL(digitalRead(6), HIGH);
}

void test_led_state_low(void) {
    digitalWrite(6, LOW);
    TEST_ASSERT_EQUAL(digitalRead(6), LOW);
}



void setup() {
    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_6_pin_number);
    pinMode(6, OUTPUT);
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() {
    if (i < max_blinks)
    {
        RUN_TEST(test_led_state_high);
        delay(500);
        RUN_TEST(test_led_state_low);
        delay(500);
        i++;
    }
    else if (i == max_blinks) {
      UNITY_END(); // stop unit testing
    }
}

#endif
