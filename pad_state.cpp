/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 20 2022 03:53:16
 */

#include "pad_state.h"
#include <assert.h>

namespace
{
    constexpr size_t MAX_PADS = 1;
    PadState inst_[MAX_PADS];
}

PadState *getPadState(size_t port)
{
    assert(port < MAX_PADS);
    return &inst_[port];
}

////////////

PadState::PadState()
{
    setAnalog(A_X, 0x80);
    setAnalog(A_Y, 0x80);
}

void PadState::setButton(ButtonID id, bool pushed)
{
    if (pushed)
    {
        pushButton(id);
    }
    else
    {
        releaseButton(id);
    }
}

void PadState::setHat(int v)
{
    static constexpr int table[] = {
        (1 << U),
        (1 << U) | (1 << R),
        (1 << R),
        (1 << D) | (1 << R),
        (1 << D),
        (1 << L) | (1 << D),
        (1 << L),
        (1 << L) | (1 << U),
    };

    auto bt = buttons_ | MASK_RLDU;
    if (v < 8)
    {
        bt ^= table[v];
    }
    buttons_ = bt;
}

void PadState::setRLDU(int x, int y)
{
    buttons_ |= MASK_RLDU;

    if (x < 64)
    {
        pushButton(L);
    }
    else if (x > 192)
    {
        pushButton(R);
    }

    if (y < 64)
    {
        pushButton(U);
    }
    else if (y > 192)
    {
        pushButton(D);
    }
}
