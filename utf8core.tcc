#pragma once

static constexpr const struct alignas(2048) Tables_s {
  enum State : uint8_t {
    ERROR_F     = 0x80,
    SURROGATE_F = 0x40,
    OVERLONG_F  = 0x20,
    DONE_F      = 0x10,

    STATE_MASK  = 0x0f,
    SHIFT_MASK  = 0x03,

    START = 0,  // (aka X__1 ...)    // (mask shift 1)
    X__2  = 1,                       // (mask shift 2)
    X__3  = 2,                       // (mask shift 3)
    X__4  = 3,                       // (mask shift 4)

    SURR  =     0 | 4 | SURROGATE_F,
    OL1   =  X__2 | 4 | OVERLONG_F,  // (mask shift 2)
    OL2_3 =  X__3 | 4,               // (mask shift 3)
    OL3_4 =  X__4 | 4,               // (mask shift 4)

    // unused:  0 | 8
    // unused:  1 | 8
    SUR_3 =  X__3 | 8,               // (mask shift 3)
    OF1_4 =  X__4 | 8,               // (mask shift 4)

    NUM_STATES = 12,

    // helper
    ERROR = START | ERROR_F,
    DONE  = START | DONE_F,
    SDONE =  DONE | SURROGATE_F,

    OX__2 =  X__2 | OVERLONG_F,
    OX__3 =  X__3 | OVERLONG_F
  };

  enum CharClass : uint8_t {
    ASC =  0 * NUM_STATES, // < 0x80

    X0F =  1 * NUM_STATES, // 0x80..0x8F  - cont1
    Y0F =  2 * NUM_STATES, // 0x90..0x9F  - cont2
    A0_ =  3 * NUM_STATES, // 0xA0..0xAF  - cont3

    C00 =  4 * NUM_STATES, // 0xC0..0xC0  - overlong1_0 (C0 80 .. C0 BF)
    C11 =  5 * NUM_STATES, // 0xC1..0xC1  - overlong1_1 (C1 80 .. C1 BF)
    C2_ =  6 * NUM_STATES, // 0xC2..0xDF  - 2byte (C2 80 .. DF BF)
    E00 =  7 * NUM_STATES, // 0xE0..0xE0  - overlong2 (E0 80 80 .. E0 9F BF), 3byte (E0 A0 80 .. E0 BF BF)
    E1C =  8 * NUM_STATES, // 0xE1..0xEC  -                                   3byte (E1 80 80 .. EC BF BF)
    EDD =  9 * NUM_STATES, // 0xED..0xED  -                                   3byte (ED 80 80 .. ED 9F BF), lead  surrogate (ED A0 80 .. ED AF BF),
                           //                                                                               trail surrogate (ED B0 80 .. ED BF BF)
    EEF = E1C,             // 0xEE..0xEF  -                                   3byte (EE 80 80 .. EF BF BF)
    F00 = 10 * NUM_STATES, // 0xF0..0xF0  - overlong3 (F0 80 80 80 .. F0 8F BF BF), 4byte (F0 90 80 80 .. F0 BF BF BF)
    F13 = 11 * NUM_STATES, // 0xF1..0xF3  -                                         4byte (F1 80 80 80 .. F3 BF BF BF)
    F44 = 12 * NUM_STATES, // 0xF4..0xF4  -                                         4byte (F4 80 80 80 .. F4 8F BF BF), overflow1
                           //                                                                                          (F4 90 80 80 .. F4 BF BF BF)
    F5F = 13 * NUM_STATES, // 0xF5..0xFF  - overflow2 (F5 80 80 80 ...)

    NUM_CLASSES = 14
  };

  const uint8_t char2class[256] = {
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x00
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x10
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x20
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x30
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x40
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x50
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x60
    ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,  ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 0x70

    X0F, X0F, X0F, X0F, X0F, X0F, X0F, X0F,  X0F, X0F, X0F, X0F, X0F, X0F, X0F, X0F, // 0x80
    Y0F, Y0F, Y0F, Y0F, Y0F, Y0F, Y0F, Y0F,  Y0F, Y0F, Y0F, Y0F, Y0F, Y0F, Y0F, Y0F, // 0x90
    A0_, A0_, A0_, A0_, A0_, A0_, A0_, A0_,  A0_, A0_, A0_, A0_, A0_, A0_, A0_, A0_, // 0xA0
    A0_, A0_, A0_, A0_, A0_, A0_, A0_, A0_,  A0_, A0_, A0_, A0_, A0_, A0_, A0_, A0_, // 0xB0
    C00, C11, C2_, C2_, C2_, C2_, C2_, C2_,  C2_, C2_, C2_, C2_, C2_, C2_, C2_, C2_, // 0xC0
    C2_, C2_, C2_, C2_, C2_, C2_, C2_, C2_,  C2_, C2_, C2_, C2_, C2_, C2_, C2_, C2_, // 0xD0
    E00, E1C, E1C, E1C, E1C, E1C, E1C, E1C,  E1C, E1C, E1C, E1C, E1C, EDD, EEF, EEF, // 0xE0
    F00, F13, F13, F13, F44, F5F, F5F, F5F,  F5F, F5F, F5F, F5F, F5F, F5F, F5F, F5F  // 0xF0
  };

  const uint8_t transitions[NUM_CLASSES * NUM_STATES] = {
  /*          START   X__2   X__3   X__4   SURR    OL1  OL2_3  OL3_4        SUR_3  OF1_4  */
  /*  ASC  */  DONE, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // ASC
  /*  X0F  */ ERROR,  DONE,  X__2,  X__3, SDONE,  DONE, OX__2, OX__3, 0, 0,  X__2,  X__3, // X0F
  /*  Y0F  */ ERROR,  DONE,  X__2,  X__3, SDONE,  DONE, OX__2,  X__3, 0, 0,  X__2, ERROR, // Y0F
  /*  A0_  */ ERROR,  DONE,  X__2,  X__3, SDONE,  DONE,  X__2,  X__3, 0, 0,  SURR, ERROR, // A0_
  /*  C00  */   OL1, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // C00
  /*  C11  */ OX__2, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // C11
  /*  C2_  */  X__2, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // C2_
  /*  E00  */ OL2_3, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // E00
  /*E1C/EEF*/  X__3, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // E1C/EEF
  /*  EDD  */ SUR_3, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // EDD
  /*  F00  */ OL3_4, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // F00
  /*  F13  */  X__4, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // F13
  /*  F44  */ OF1_4, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR, // F44
  /*  F5F  */ ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 0, 0, ERROR, ERROR  // F5F
  };
} Tables{};

