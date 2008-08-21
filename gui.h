#ifndef _H_BITTWIT_GUI_H_
#define _H_BITTWIT_GUI_H_

#include <gtk/gtk.h>

#define MAXBYTEROWS 32

typedef enum { RT_ByteRow, RT_DelayRow } RowType;

//  vb_  => vbox
//  hb_  => hbox
//  btn_ => button
//  cb_  => checkbox
//  lbl_ => label
//  ent_ => entry box (textbox)
typedef struct
{
    GtkWidget* hb_outer;        // holds the widgets for the whole row
    GtkWidget* cb_sendbyte;     // Send This Byte checkbox
    GtkWidget* btn_insert;      // Insert a New Row Here button
    GtkWidget* btn_remove;      // Remove This Row button
    GtkWidget* btn_delay;       // Switch to Delay Row button
    GtkWidget* hb_bytebox;      // holds the widgets for RT_ByteRow type
    GtkWidget* cb_bitset[8];    // array of Set This Bit checkboxes
    GtkWidget* lbl_hex;         // hexadecimal representation of byte
    GtkWidget* hb_delaybox;     // holds the widgets for RT_DelayRow type
    GtkWidget* ent_delay;       // entry widget for delay time (in ms)

    RowType type;               // what kind of info are we asking for?
	int bytevalue;
} ByteRow, *PByteRow;
typedef struct
{
    GtkWidget* mw;                 // main window
    GtkWidget* vb_outer;           // holds the major areas of the window
    GtkWidget* vb_workarea;        // area with the bit boxes
    GtkWidget* hb_buttonrow;       // button row along bottom of window
	GtkWidget* btn_send;           // Send button in button row
    GtkWidget* btn_done;           // Done button in button row
    GtkWidget* btn_addrow;         // Add Row button in button row
    ByteRow byterow[MAXBYTEROWS];  // array of rows for each byte

    int byterows;  // number of rows that we have provided to user
} BTWnd, *PBTWnd;

PBTWnd create_mw();

#endif // _H_BITTWIT_GUI_H_
