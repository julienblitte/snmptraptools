#ifndef PLUGIN_SMTP_RESOURCES_H_INCLUDED
#define PLUGIN_SMTP_RESOURCES_H_INCLUDED

#define ID_DIALOG_CONFIG   1

#define ID_EDIT_SERVER		0x401
#define ID_EDIT_FROM		0x402
#define ID_EDIT_TO			0x403
#define ID_EDIT_LOGIN		0x404
#define ID_EDIT_PASSWORD	0x405

#define ID_BUTTON_OK			0x101
#define ID_BUTTON_CANCEL		0x102

#define ID_GROUPBOX_AUTH		0x201

#define ID_CHECKBOX_AUTH		0x301

#define ON_ITEM_SELECT (( LBN_SELCHANGE )<<16)
#define ON_CLICK       (( BN_CLICKED    )<<16)
#define ON_CHANGE      (( EN_CHANGE     )<<16)
#define ON_CHECK		(( 0     )<<16)

#endif // PLUGIN_SMTP_RESOURCES_H_INCLUDED
