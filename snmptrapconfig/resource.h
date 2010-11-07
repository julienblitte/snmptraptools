#include <windows.h>

// events
#define ON_ITEM_SELECT (( LBN_SELCHANGE )<<16)
#define ON_CLICK       (( BN_CLICKED    )<<16)
#define ON_CHANGE      (( EN_CHANGE     )<<16)


#define ID_DIALOG_MAIN 1

    #define ID_BUTTON_START     0x101
    #define ID_BUTTON_STOP      0x102
    #define ID_BUTTON_ADD       0x105
    #define ID_BUTTON_REMOVE    0x106
    #define ID_BUTTON_MODIFY    0x107
    #define ID_BUTTON_UPDATE    0x108
    #define ID_BUTTON_EXPORT    0x109

    #define ID_BROWSE_RUN       0x120
    #define ID_BROWSE_WORK_DIR  0x121

    #define ID_LISTBOX_OID      0x201

    #define ID_EDIT_TRAP_CODE    0x401
    #define ID_EDIT_RUN          0x402
    #define ID_EDIT_WORK_DIR     0x403
    #define ID_EDIT_DESCRIPTION  0x404

    #define ID_GROUPBOX_TRAPTYPE   0x301

    #define ID_RADIO_ANY          0x501
    #define ID_RADIO_GENERIC      0x502
    #define ID_RADIO_SPECIFIC     0x503

    #define ID_CAPTION_TRAP_CODE  0x601


#define ID_DIALOG_ADD   2

    #define ID_EDIT_OID      0x410

    #define ID_BUTTON_ADD_OID    0x110
    #define ID_BUTTON_CANCEL_OID 0x111

#define ID_ICON         1
#define ID_VERSION_INFO 1
