// all of this is in 9.6.1

#ifndef OFFSETS_H
#define OFFSETS_H

#ifdef __aarch64__

constexpr uintptr_t OFF_G_DisplayInfo          = 0x2593760;
constexpr uintptr_t DISPLAYINFO_SCREEN_WIDTH   = 244;   // *(int*)(app + 244)
constexpr uintptr_t DISPLAYINFO_SCREEN_HEIGHT  = 248;   // *(int*)(app + 248)


constexpr uintptr_t OFF_PersistSave            = 0x14E37EC;
constexpr uintptr_t OFF_PersistReadBool        = 0x14E3CC8;
constexpr uintptr_t OFF_PersistManager         = 0x2599998;

constexpr uintptr_t OFF_SettingsCreate         = 0xA0FF78;
constexpr uintptr_t OFF_CheckboxCreate         = 0xA10688;
constexpr uintptr_t OFF_SettingsAddWidget      = 0xA10244;
constexpr uintptr_t OFF_SettingsDispatch       = 0xA145AC;
constexpr uintptr_t OFF_SettingsDialogPopulate = 0xA0EC6C;
constexpr uintptr_t OFF_SettingsContentCreate  = 0x12B9E5C;

constexpr uintptr_t SETTINGS_PAGE_OWNER        = 192;   // 0xC0
constexpr uintptr_t SETTINGS_PAGE_DIRTY        = 292;   // 0x124

// ID map - do not reuse
// Tabs created by sub_A0EC6C: 3,6,7,8,9,10,12,15,18,21,24,26
// Sliders: 4,5
// Checkboxes (via sub_A145AC): 13,14,16,17,19,20,27,29

constexpr uint32_t SETTINGS_VIEW_ANGLE_ID = 30;
constexpr uint32_t CHECKBOX_VIEW_HIGH_ID  = 31;
constexpr uint32_t CHECKBOX_VIEW_LOW_ID   = 32;

constexpr uintptr_t CONFIG_USE_HIGH_VIEW_ANGLE = 2582;  // 0xA16
constexpr uintptr_t CONFIG_USAGE_SHARING       = 2579;  // 0xA13 (org)

// constexpr uintptr_t OFF_ZoomSubtractor = 0xB4E894; unused
constexpr uintptr_t OFF_BoardZoom2 = 0xAA0C40;

constexpr uintptr_t BOARD_280 = 1120;   // 0x460 
constexpr uintptr_t BOARD_281 = 1124;   // 0x464
constexpr uintptr_t BOARD_283 = 1132;   // 0x46C
constexpr uintptr_t BOARD_284 = 1136;   // 0x470
constexpr uintptr_t BOARD_285 = 1140;   // 0x474
constexpr uintptr_t BOARD_286 = 1144;   // 0x478
constexpr uintptr_t BOARD_270 = 1080;   // 0x438

constexpr uintptr_t OFF_IconLoader       = 0x5E7AA4;
constexpr uintptr_t OFF_IconResourceN    = 0x2576AA0;
constexpr uintptr_t OFF_IconResourceS    = 0x2576AC8;
constexpr uintptr_t SETTINGS_PAGE_CONTAINER = 240;

constexpr uintptr_t OFF_SettingsStringCreate = 0x5B5B84;

#endif

#endif
