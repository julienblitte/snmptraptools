#include <windows.h>

// events
#define ON_LIST_SELECT (( LBN_SELCHANGE )<<16)
#define ON_CLICK       (( BN_CLICKED    )<<16)
#define ON_CHANGE      (( EN_CHANGE     )<<16)
#define ON_COMBO_SELECT (( CBN_SELCHANGE     )<<16)
#define ON_MENU_CLICK	(( 0     )<<16)

#define ID_DIALOG_MAIN 1

    #define ID_BUTTON_ADD       0x101
    #define ID_BUTTON_REMOVE    0x102
    #define ID_BUTTON_MODIFY    0x103

    #define ID_LISTBOX_OID      0x201

    #define ID_EDIT_TRAP_CODE    0x401
    #define ID_EDIT_DESCRIPTION  0x404

    #define ID_GROUPBOX_TRAPTYPE   0x301
    #define ID_COMBO_PLUGIN		0x302

    #define ID_RADIO_ANY          0x501
    #define ID_RADIO_GENERIC      0x502
    #define ID_RADIO_SPECIFIC     0x503

    #define ID_CAPTION_TRAP_CODE  0x601

    #define ID_MENU_DIALOG_1	0x900
    #define ID_MENU_EXPORT		0x901
    #define ID_MENU_EXIT		0x902
    #define ID_MENU_START		0x903
    #define ID_MENU_STOP		0x904
    #define ID_MENU_UPDATE		0x905
    #define ID_MENU_PLUGIN		0x906


#define ID_DIALOG_ADD   2

    #define ID_EDIT_OID      0x410

    #define ID_BUTTON_ADD_OID    0x110
    #define ID_BUTTON_CANCEL_OID 0x111

#define ID_ICON         1
#define ID_VERSION_INFO 1
