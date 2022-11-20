/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 20 2022 03:50:18
 */
#pragma once

#include <stdint.h>
#include <stdlib.h>

class PadState
{
public:
    enum ButtonID
    {
        R = 7,
        L = 6,
        D = 5,
        U = 4,
        START = 3,
        A = 2,
        C = 1,
        B = 0,
        RT = 15,
        X = 14,
        Y = 13,
        Z = 12,
        LT = 11,
    };

    static constexpr int MASK_RLDU = 0b11110000;

    enum AnalogID
    {
        A_X,
        A_Y,
        A_RT,
        A_LT,
    };

public:
    PadState();

    uint16_t getButtons() const { return buttons_; }
    uint8_t getAnalog(AnalogID ch) const { return analog_[ch]; }

    void releaseButton(ButtonID id) { buttons_ |= 1 << id; }
    void pushButton(ButtonID id) { buttons_ &= ~(1 << id); }
    void setButton(ButtonID id, bool pushed);
    void setAnalog(AnalogID id, int v) { analog_[id] = v; }

    void setHat(int v);
    void setRLDU(int x, int y);

private:
    uint16_t buttons_{0xffff};
    uint8_t analog_[4]{};
};

PadState *getPadState(size_t port = 0);