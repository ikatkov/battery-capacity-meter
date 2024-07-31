// All segments
//  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
const uint8_t TEXT_ON1[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0,                                             // space
    SEG_B | SEG_C                                  // 1
};

const uint8_t TEXT_ON2[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0,                                             // space
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G          // 2
};

const uint8_t TEXT_ON3[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0,                                             // space
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G          // 3
};

const uint8_t TEXT_ON4[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0,                                             // space
    SEG_B | SEG_C | SEG_F | SEG_G                  // 4
};

const uint8_t TEXT_ON5[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0,                                             // space
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G                  // 5
};

const uint8_t TEXT_CHRG[] = {
    SEG_A | SEG_D | SEG_E | SEG_F,         // C
    SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // H
    SEG_E | SEG_G,                         // r
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F  // G
};

const uint8_t TEXT_ONLY[] = {
    SEG_C | SEG_D | SEG_E | SEG_G, // o
    SEG_C | SEG_E | SEG_G,         // n
    SEG_B | SEG_C,                 // l
    0                              // space
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

// Overvoltage, either from the charger or the battery
const uint8_t TEXT_ERR_1_OVERVOLTAGE[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_E | SEG_G,                         // r
    SEG_E | SEG_G,                         // r
    SEG_B | SEG_C                          // 1
};

// Battery disconnected mid charge process
const uint8_t TEXT_ERR_2_BATT_DISCONNECTD_MID_CHARGE[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_E | SEG_G,                         // r
    SEG_E | SEG_G,                         // r
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G  // 2
};

// Battery disconnected mid discharge process
const uint8_t TEXT_ERR_3_BATT_DISCONNECTD_MID_DISCHARGE[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_E | SEG_G,                         // r
    SEG_E | SEG_G,                         // r
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G  // 3
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

const uint8_t TEXT_STOR[] = {
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G, // S
    SEG_D | SEG_E | SEG_F | SEG_G,         // t
    SEG_C | SEG_D | SEG_E | SEG_G,         // o
    SEG_E | SEG_G                          // r
};