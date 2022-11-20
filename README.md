CyberStickUSB2SegaSaturn
----
サイバースティック(USB)をセガサターンに接続するためのアダプタです。

    SS_VCC(1) - PICO_VBUS(40)
    SS_D1(2)  - PICO_GP2(4)  - 18K - GND
    SS_D0(3)  - PICO_GP3(5)  - 18K - GND
    SS_TH(4)  - 10K  - PICO_GP4(6) - 18K - GND
    SS_TR(5)  - 4.7K - PICO_GP5(7) - 18K - GND
    SS_TL(6)  - PICO_GP6(9)  - 18K - GND
    SS_D3(7)  - PICO_GP7(10) - 18K - GND
    SS_D2(8)  - PICO_GP8(11) - 18K - GND
    SS_GND(9) - PICO_GND(3, 8, 13, ...)

セガサターンに接続しつつPC等のUSBに接続したりすると、どちらかにダメージを与える可能性があるので気をつけてください。
