#pragma once

#define CMD_DATE_TIME              0x01
#define CMD_WEATHER                0x02
#define CMD_TEMP_HUMIDITY          0x03
#define CMD_VOC_NOX                0x04
#define CMD_LOG                    0x05
#define CMD_BATTERY                0x07
#define CMD_LIGHT                  0x08

#define CMD_VOICE_START            0x10
#define CMD_VOICE_END              0x11
#define CMD_VOICE_CANCEL           0x12
#define CMD_VOICE_UPSTATUS         0x13

#define CMD_PLAYER_LAST_REQ        0x30
#define CMD_PLAYER_LAST_RESP       0x31
#define CMD_PLAYER_LOAD_AND_PLAY   0x32
#define CMD_PLAYER_PLAY            0x33
#define CMD_PLAYER_PAUSE           0x34
#define CMD_PLAYER_SEEK            0x35
#define CMD_PLAYER_OPEN_DIR        0x36
#define CMD_PLAYER_FILES_REQ       0x37
#define CMD_PLAYER_FILES_RESP      0x38

#define CMD_SETTING_ENTER          0x40
#define CMD_SETTING_EXIT           0x41
#define CMD_SETTING_URL            0x42

#define CMD_WIFI_AP                0x50

#define CMD_SET_STATUS             0x60
#define CMD_SET_NOTIFICATION       0x61
#define CMD_SET_EMOTION            0x62
#define CMD_SET_CHAT_MESSAGE       0x63

#define CMD_LCD_LOG                0x90