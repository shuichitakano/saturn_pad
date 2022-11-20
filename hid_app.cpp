/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 20 2022 03:32:25
 */

#include <tusb.h>
#include <stdio.h>
#include "pad_state.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_REPORT 4

    namespace
    {
        uint8_t _report_count[CFG_TUH_HID];
        tuh_hid_report_info_t _report_info_arr[CFG_TUH_HID][MAX_REPORT];

        bool isDS4(uint16_t vid, uint16_t pid)
        {
            return vid == 0x054c && (pid == 0x09cc || pid == 0x05c4);
        }

        bool isDS5(uint16_t vid, uint16_t pid)
        {
            return vid == 0x054c && pid == 0x0ce6;
        }

        struct DS4Report
        {
            // https://www.psdevwiki.com/ps4/DS4-USB

            struct Button1
            {
                inline static constexpr int SQUARE = 1 << 4;
                inline static constexpr int CROSS = 1 << 5;
                inline static constexpr int CIRCLE = 1 << 6;
                inline static constexpr int TRIANGLE = 1 << 7;
            };

            struct Button2
            {
                inline static constexpr int L1 = 1 << 0;
                inline static constexpr int R1 = 1 << 1;
                inline static constexpr int L2 = 1 << 2;
                inline static constexpr int R2 = 1 << 3;
                inline static constexpr int SHARE = 1 << 4;
                inline static constexpr int OPTIONS = 1 << 5;
                inline static constexpr int L3 = 1 << 6;
                inline static constexpr int R3 = 1 << 7;
            };

            uint8_t reportID;
            uint8_t stickL[2];
            uint8_t stickR[2];
            uint8_t buttons1;
            uint8_t buttons2;
            uint8_t ps : 1;
            uint8_t tpad : 1;
            uint8_t counter : 6;
            uint8_t triggerL;
            uint8_t triggerR;
            // ...

            int getHat() const { return buttons1 & 15; }

            void translate(PadState *pad) const
            {
                pad->setAnalog(PadState::A_X, stickL[0]);
                pad->setAnalog(PadState::A_Y, stickL[1]);
                pad->setAnalog(PadState::A_LT, triggerL);
                pad->setAnalog(PadState::A_RT, triggerR);
                pad->setButton(PadState::START, buttons2 & Button2::OPTIONS);
                pad->setButton(PadState::A, buttons1 & Button1::CROSS);
                pad->setButton(PadState::B, buttons1 & Button1::CIRCLE);
                pad->setButton(PadState::C, buttons2 & Button2::L1);
                pad->setButton(PadState::X, buttons1 & Button1::SQUARE);
                pad->setButton(PadState::Y, buttons1 & Button1::TRIANGLE);
                pad->setButton(PadState::Z, buttons2 & Button2::R1);
                pad->setHat(getHat());
            }
        };

        struct DS5Report
        {
            uint8_t reportID;
            uint8_t stickL[2];
            uint8_t stickR[2];
            uint8_t triggerL;
            uint8_t triggerR;
            uint8_t counter;
            uint8_t buttons[3];
            // ...

            struct Button
            {
                inline static constexpr int SQUARE = 1 << 4;
                inline static constexpr int CROSS = 1 << 5;
                inline static constexpr int CIRCLE = 1 << 6;
                inline static constexpr int TRIANGLE = 1 << 7;
                inline static constexpr int L1 = 1 << 8;
                inline static constexpr int R1 = 1 << 9;
                inline static constexpr int L2 = 1 << 10;
                inline static constexpr int R2 = 1 << 11;
                inline static constexpr int SHARE = 1 << 12;
                inline static constexpr int OPTIONS = 1 << 13;
                inline static constexpr int L3 = 1 << 14;
                inline static constexpr int R3 = 1 << 15;
                inline static constexpr int PS = 1 << 16;
                inline static constexpr int TPAD = 1 << 17;
            };

            int getHat() const { return buttons[0] & 15; }
            int getButtons() const
            {
                return buttons[0] | (buttons[1] << 8) | (buttons[2] << 16);
            }

            void translate(PadState *pad) const
            {
                auto bt = getButtons();
                pad->setAnalog(PadState::A_X, stickL[0]);
                pad->setAnalog(PadState::A_Y, stickL[1]);
                pad->setAnalog(PadState::A_LT, triggerL);
                pad->setAnalog(PadState::A_RT, triggerR);
                pad->setButton(PadState::START, bt & Button::OPTIONS);
                pad->setButton(PadState::A, bt & Button::CROSS);
                pad->setButton(PadState::B, bt & Button::CIRCLE);
                pad->setButton(PadState::C, bt & Button::L1);
                pad->setButton(PadState::X, bt & Button::SQUARE);
                pad->setButton(PadState::Y, bt & Button::TRIANGLE);
                pad->setButton(PadState::Z, bt & Button::R1);
                pad->setHat(getHat());
            }
        };
    }

    void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
    {
        uint16_t vid, pid;
        tuh_vid_pid_get(dev_addr, &vid, &pid);

        printf("HID device address = %d, instance = %d is mounted\n", dev_addr, instance);
        printf("VID = %04x, PID = %04x\r\n", vid, pid);

        const char *protocol_str[] = {"None", "Keyboard", "Mouse"}; // hid_protocol_type_t
        uint8_t const interface_protocol = tuh_hid_interface_protocol(dev_addr, instance);

        // Parse report descriptor with built-in parser
        _report_count[instance] = tuh_hid_parse_report_descriptor(_report_info_arr[instance], MAX_REPORT, desc_report, desc_len);
        printf("HID has %u reports and interface protocol = %d:%s\n", _report_count[instance],
               interface_protocol, protocol_str[interface_protocol]);

        if (!tuh_hid_receive_report(dev_addr, instance))
        {
            printf("Error: cannot request to receive report\r\n");
        }
    }

    void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
    {
        printf("HID device address = %d, instance = %d is unmounted\n", dev_addr, instance);
    }

    void tuh_hid_report_received_cb(uint8_t dev_addr,
                                    uint8_t instance, uint8_t const *report, uint16_t len)
    {
        uint8_t const rpt_count = _report_count[instance];
        tuh_hid_report_info_t *rpt_info_arr = _report_info_arr[instance];
        tuh_hid_report_info_t *rpt_info = NULL;

        uint16_t vid, pid;
        tuh_vid_pid_get(dev_addr, &vid, &pid);

        if (isDS4(vid, pid))
        {
            if (sizeof(DS4Report) <= len)
            {
                auto r = reinterpret_cast<const DS4Report *>(report);
                if (r->reportID != 1)
                {
                    printf("Invalid reportID %d\n", r->reportID);
                    return;
                }

                auto *pad = getPadState();
                assert(pad);
                r->translate(pad);
            }
            else
            {
                printf("Invalid DS4 report size %zd\n", len);
                return;
            }
        }
        else if (isDS5(vid, pid))
        {
            if (sizeof(DS5Report) <= len)
            {

                auto r = reinterpret_cast<const DS5Report *>(report);
                if (r->reportID != 1)
                {
                    printf("Invalid reportID %d\n", r->reportID);
                    return;
                }

                auto *pad = getPadState();
                assert(pad);
                r->translate(pad);
            }
            else
            {
                printf("Invalid DS5 report size %zd\n", len);
                return;
            }
        }
        else
        {
            if (rpt_count == 1 && rpt_info_arr[0].report_id == 0)
            {
                // Simple report without report ID as 1st byte
                rpt_info = &rpt_info_arr[0];
            }
            else
            {
                // Composite report, 1st byte is report ID, data starts from 2nd byte
                uint8_t const rpt_id = report[0];

                // Find report id in the arrray
                for (uint8_t i = 0; i < rpt_count; i++)
                {
                    if (rpt_id == rpt_info_arr[i].report_id)
                    {
                        rpt_info = &rpt_info_arr[i];
                        break;
                    }
                }

                report++;
                len--;
            }

            if (!rpt_info)
            {
                printf("Couldn't find the report info for this report !\n");
                return;
            }

            //        printf("usage %d, %d\n", rpt_info->usage_page, rpt_info->usage);

            if (rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP)
            {
                switch (rpt_info->usage)
                {
                case HID_USAGE_DESKTOP_KEYBOARD:
                    TU_LOG1("HID receive keyboard report\n");
                    // Assume keyboard follow boot report layout
                    //                process_kbd_report((hid_keyboard_report_t const *)report);
                    break;

                case HID_USAGE_DESKTOP_MOUSE:
                    TU_LOG1("HID receive mouse report\n");
                    // Assume mouse follow boot report layout
                    //                process_mouse_report((hid_mouse_report_t const *)report);
                    break;

                case HID_USAGE_DESKTOP_JOYSTICK:
                {
                    // TU_LOG1("HID receive joystick report\n");
                    struct JoyStickReport
                    {
                        uint8_t axis[3];
                        uint8_t buttons;
                        // 実際のところはしらん

                        void translateBGCFC801(PadState *dst) const
                        {
                            dst->setAnalog(PadState::A_X, axis[0]);
                            dst->setAnalog(PadState::A_Y, axis[1]);
                            dst->setButton(PadState::START, buttons & (1 << 7));
                            dst->setButton(PadState::A, buttons & (1 << 0));
                            dst->setButton(PadState::B, buttons & (1 << 1));
                            // todo
                            dst->setRLDU(axis[0], axis[1]);
                        }
                    };
                    auto *rep = reinterpret_cast<const JoyStickReport *>(report);
                    //                printf("x %d y %d button %02x\n", rep->axis[0], rep->axis[1], rep->buttons);

                    auto *pad = getPadState();
                    assert(pad);

                    rep->translateBGCFC801(pad);

                    // BUFFALO BGC-FC801
                    // VID = 0411, PID = 00c6
                }
                break;

                case HID_USAGE_DESKTOP_GAMEPAD:
                {
                    // TU_LOG1("HID receive gamepad report\n");
                    struct GamePadReport
                    {
                        uint16_t buttons;
                        uint8_t hat;
                        uint8_t axis[4];
                        // 実際のところはしらん

                        void translate(PadState *dst) const
                        {
                            dst->setAnalog(PadState::A_X, axis[0]);
                            dst->setAnalog(PadState::A_Y, axis[1]);
                            dst->setAnalog(PadState::A_RT, axis[3]);
                            dst->setButton(PadState::START, buttons & (1 << 7));
                            dst->setButton(PadState::A, buttons & (1 << 1));
                            dst->setButton(PadState::B, buttons & (1 << 0));
                            dst->setButton(PadState::C, buttons & (1 << 2));
                            // dst->setRLDU(axis[0], axis[1]);
                        }
                    };
                    auto *rep = reinterpret_cast<const GamePadReport *>(report);
                    // auto *rep = reinterpret_cast<const hid_gamepad_report_t *>(report);

                    auto *pad = getPadState();
                    assert(pad);

                    rep->translate(pad);

                    // printf("(%d %d %d %d) %08x\n", rep->axis[0], rep->axis[1], rep->axis[2], rep->axis[3], rep->buttons);
                }
                break;

                default:
                    break;
                }
            }
        }

        if (!tuh_hid_receive_report(dev_addr, instance))
        {
            printf("Error: cannot request to receive report\r\n");
        }
    }

#ifdef __cplusplus
}
#endif