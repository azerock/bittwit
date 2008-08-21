#include <stdlib.h>  // for malloc
#include <stdio.h>  // for sprintf
#include <string.h> // for memmove
#include <errno.h>
#include "gui.h"  // includes <gtk/gtk.h>

#define MW_WIDTH  600
#define MW_HEIGHT 500
#define INITIALBYTEROWS 4
#define DEFAULTDELAY 50  // in ms
#define MINDELAY 0
#define MAXDELAY 60000

const gchar* Key_RowIdx = "RowIdx";
const gchar* Key_BitIdx = "BitIdx";

// Event handler definitions
static gboolean ev_mw_delete(GtkWidget* w, GdkEvent* event, gpointer data);
static gboolean ev_send_clicked(GtkButton* btn, gpointer data);
static gboolean ev_done_clicked(GtkButton* btn, gpointer data);
static gboolean ev_addrow_clicked(GtkButton* btn, gpointer data);
static gboolean ev_insert_clicked(GtkButton* btn, gpointer data);
static gboolean ev_remove_clicked(GtkButton* btn, gpointer data);
static gboolean ev_delay_clicked(GtkButton* btn, gpointer data);
static gboolean ev_sendbyte_toggled(GtkToggleButton* btn, gpointer data);
static gboolean ev_bitset_toggled(GtkToggleButton* btn, gpointer data);

GtkWidget* wrap_hbox(GtkWidget* w)
{
	GtkWidget* b = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(b);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 0);
	return(b);
}

GtkWidget* wrap_vbox(GtkWidget* w)
{
	GtkWidget* b = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(b);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 0);
	return(b);
}

GtkWidget* wrap_hvboxes(GtkWidget* w)
{
	return(wrap_hbox(wrap_vbox(w)));
}

void update_hexvalue(PByteRow pbr)
{
	int value = 0;
	int idx;
	char buffer[5];

	for (idx = 0; idx < 8; ++idx) {
		GtkWidget* cb = pbr->cb_bitset[idx];
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb)))
			value |= (1 << idx);
	}

	// save
	pbr->bytevalue = value;

	sprintf(buffer, "0x%02X", value);
	gtk_label_set_text(GTK_LABEL(pbr->lbl_hex), buffer);
}

GtkWidget* create_checkbox(GtkWidget* b, void* callback, PBTWnd pbtwnd)
{
	GtkWidget* w = gtk_check_button_new();
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(callback), pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(b), w, FALSE, FALSE, 0);
	return(w);
}

void set_entry_text_to_int(GtkEntry* ent, int value)
{
	char buffer[16];
	sprintf(buffer, "%i", value);
	gtk_entry_set_text(ent, buffer);
}
int get_entry_text_as_int(GtkEntry* ent)
{
	int value;
	const gchar * buffer = gtk_entry_get_text(ent);
	errno = 0;
	value = strtol(buffer, NULL, 10);
	if (errno || value < MINDELAY || value > MAXDELAY) {
		// TODO: yell at user
		return(DEFAULTDELAY);
	}
	return(value);
}

void create_byterow(PBTWnd pbtwnd, int br_idx)
{
	// NOTES:
	// To the event handlers, pass pbtwnd
	// Put enough info in userdata to be able to find needed info in pbtwnd

	GtkWidget* w;
	GtkWidget* b;
	int bitIdx;

	PByteRow pbr = &pbtwnd->byterow[br_idx];

	// We always need to increment this counter here. If we're adding a row
	// to the end, the need seems clear. If we're inserting a row in the
	// middle, leave it to this procedure to account for it
	++pbtwnd->byterows;

	// The default for every new byterow is RT_ByteRow type
	pbr->type = RT_ByteRow;
	pbr->bytevalue = 0;

	// byte row
	// the gtk_box_reorder call is needed when were are inserting a row
	// in the middle of the list. most of the time, adding the row to
	// the end is sufficient
	w = gtk_hbox_new(FALSE, 10);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbtwnd->vb_workarea), w, FALSE, FALSE, 0);
	if (br_idx + 1 < pbtwnd->byterows)
		gtk_box_reorder_child(GTK_BOX(pbtwnd->vb_workarea), w, br_idx);
	pbr->hb_outer = w;
	
	// "send this byte" checkbox
	w = create_checkbox(pbr->hb_outer, ev_sendbyte_toggled, pbtwnd);
	g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
	pbr->cb_sendbyte = w;

	// temp box to hold insert, remove, and delay buttons
	b = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(b);
	gtk_box_pack_start(GTK_BOX(pbr->hb_outer), b, FALSE, FALSE, 0);

	// Insert button
	w = gtk_button_new_with_label("I");
	g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(ev_insert_clicked), pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 2);
	pbr->btn_insert = w;

	// Remove button
	w = gtk_button_new_with_label("R");
	g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(ev_remove_clicked), pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 2);
	pbr->btn_remove = w;

	// Delay button
	w = gtk_button_new_with_label("Y");
	g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
	g_signal_connect(G_OBJECT(w),"clicked",G_CALLBACK(ev_delay_clicked),pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 2);
	pbr->btn_delay = w;

	// bytebox - When type is RT_ByteRow, this box is visible
	w = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbr->hb_outer), w, TRUE, FALSE, 0);
	pbr->hb_bytebox = w;

	// temp box to hold all the bit checkboxes
	b = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(b);
	gtk_box_pack_start(GTK_BOX(pbr->hb_bytebox), b, TRUE, FALSE, 0);

	// 4 boxes, a separator, 4 more boxes
	for (bitIdx = 0; bitIdx < 4; ++bitIdx) {
		w = create_checkbox(b, ev_bitset_toggled, pbtwnd);
		g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
		g_object_set_data(G_OBJECT(w), Key_BitIdx, (gpointer)bitIdx);
		pbr->cb_bitset[7 - bitIdx] = w;
	}
	w = gtk_vseparator_new();
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(b), w, TRUE, FALSE, 0);
	for ( ; bitIdx < 8; ++bitIdx) {
		w = create_checkbox(b, ev_bitset_toggled, pbtwnd);
		g_object_set_data(G_OBJECT(w), Key_RowIdx, (gpointer)br_idx);
		g_object_set_data(G_OBJECT(w), Key_BitIdx, (gpointer)bitIdx);
		pbr->cb_bitset[7 - bitIdx] = w;
	}

	w = gtk_label_new("");
	gtk_widget_set_size_request(w, 45, -1);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbr->hb_bytebox), w, TRUE, FALSE, 0);
	pbr->lbl_hex = w;

	// delaybox - When type is RT_DelayRow, this box is visible
	w = gtk_hbox_new(FALSE, 0);
	//gtk_widget_show(w);  // not visible by default
	gtk_box_pack_start(GTK_BOX(pbr->hb_outer), w, TRUE, FALSE, 0);
	pbr->hb_delaybox = w;

	w = gtk_label_new("Delay time (in ms):");
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbr->hb_delaybox), w, FALSE, FALSE, 0);
	
	// delay entry box
	w = gtk_entry_new_with_max_length(5);  // max length == 5 digits
	gtk_widget_set_size_request(w, 60, -1);
	set_entry_text_to_int(GTK_ENTRY(w), DEFAULTDELAY);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbr->hb_delaybox), w, FALSE, FALSE, 5);
	pbr->ent_delay = w;
}
void destroy_byterow(PBTWnd pbtwnd, int br_idx)
{
	--pbtwnd->byterows;
	gtk_widget_destroy(pbtwnd->byterow[br_idx].hb_outer);
}

void move_byterow(PByteRow source, PByteRow dest, int new_idx)
{
	// Move the data as-is
	memmove(dest, source, sizeof(ByteRow));

	// Update the byte row in the object data for all user-responsive
	// widgets
	int i;
	GtkWidget* w[] = { 0, 0, 0, 0, 0, 0, 0, 0, dest->cb_sendbyte, dest->btn_insert, dest->btn_remove, dest->btn_delay };
	for (i = 0; i < 8; ++i)
		w[i] = dest->cb_bitset[i];
	for (i = 0; i < 8 + 4; ++i)
		g_object_set_data(G_OBJECT(w[i]), Key_RowIdx, (gpointer)new_idx);
}

void insert_byterow(PBTWnd pbtwnd, int br_idx)
{
	int r;

	// Move all rows from 'br_idx' up one
	for (r = pbtwnd->byterows; r > br_idx; --r) {
		PByteRow source = &pbtwnd->byterow[r-1];
		PByteRow dest = &pbtwnd->byterow[r];
		move_byterow(source, dest, r);
	}

	create_byterow(pbtwnd, br_idx);
}
void remove_byterow(PBTWnd pbtwnd, int br_idx)
{
	destroy_byterow(pbtwnd, br_idx);

	// Move all rows after the deleted row up one
	for ( ; br_idx < pbtwnd->byterows; ++br_idx) {
		PByteRow source = &pbtwnd->byterow[br_idx+1];
		PByteRow dest = &pbtwnd->byterow[br_idx];
		move_byterow(source, dest, br_idx);
	}
}

void toggle_byterow_type(PByteRow pbr)
{
	if (pbr->type == RT_ByteRow) {
		pbr->type = RT_DelayRow;
		gtk_widget_hide(pbr->hb_bytebox);
		gtk_widget_show(pbr->hb_delaybox);

		// when switching byterow->delayrow, assume user wants
		// delay included and tick the box. Will have no effect
		// if the box is already ticked
		GtkToggleButton* tb = GTK_TOGGLE_BUTTON(pbr->cb_sendbyte);
		gtk_toggle_button_set_active(tb, TRUE);
	} else {
		pbr->type = RT_ByteRow;
		gtk_widget_hide(pbr->hb_delaybox);
		gtk_widget_show(pbr->hb_bytebox);
	}
}

void create_workarea(PBTWnd pbtwnd)
{
	GtkWidget* w;
	GtkWidget* c;
	int row_idx;

	// scrollable container
	c = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(c), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_show(c);
	gtk_box_pack_start(GTK_BOX(pbtwnd->vb_outer), c, TRUE, TRUE, 0);

	// work area vbox
	w = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(w);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(c), w);
	pbtwnd->vb_workarea = w;

	// create byterows
	for (row_idx = 0; row_idx < INITIALBYTEROWS; row_idx++)
		create_byterow(pbtwnd, row_idx);
}

void create_buttonrow(PBTWnd pbtwnd)
{
	GtkWidget* w;

	// button row
	w = gtk_hbox_new(TRUE, 10);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbtwnd->vb_outer), w, FALSE, FALSE, 10);
	pbtwnd->hb_buttonrow = w;

	w = gtk_button_new_with_label("Send");
	g_signal_connect(G_OBJECT(w),"clicked",G_CALLBACK(ev_send_clicked),pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbtwnd->hb_buttonrow), w, TRUE, FALSE, 0);
	pbtwnd->btn_send = w;

	// Add Row button
	w = gtk_button_new_with_label("Add Row");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(ev_addrow_clicked), pbtwnd);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbtwnd->hb_buttonrow), w, TRUE, FALSE, 0);
	pbtwnd->btn_addrow = w;

	// Done button
	w = gtk_button_new_with_label("Done");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(ev_done_clicked), NULL);
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(pbtwnd->hb_buttonrow), w, TRUE, FALSE, 0);
	pbtwnd->btn_done = w;
}

PBTWnd create_mw()
{
	GtkWidget* w;

	PBTWnd pbtwnd = malloc(sizeof(BTWnd));
	if (!pbtwnd)
		return(NULL);

	// set any non-control-handle defaults here
	pbtwnd->byterows = 0;

	// main window
	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(w), "BT");
	gtk_widget_set_size_request(w, MW_WIDTH, MW_HEIGHT);
	gtk_window_set_resizable(GTK_WINDOW(w), FALSE);
	g_signal_connect(G_OBJECT(w),"delete_event",G_CALLBACK(ev_mw_delete),NULL);
	gtk_container_set_border_width(GTK_CONTAINER(w), 10);
	pbtwnd->mw = w;

	// outer vbox
	w = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(w);
	gtk_container_add(GTK_CONTAINER(pbtwnd->mw), w);
	pbtwnd->vb_outer = w;

	create_workarea(pbtwnd);
	create_buttonrow(pbtwnd);

	return (pbtwnd);
}

// Event handlers
static gboolean ev_mw_delete(GtkWidget* w, GdkEvent* event, gpointer data)
{
	gtk_main_quit();
	return(FALSE);
}

static gboolean ev_send_clicked(GtkButton* btn, gpointer data)
{
	// TODO
	return(FALSE);
}

static gboolean ev_done_clicked(GtkButton* btn, gpointer data)
{
	gtk_main_quit();
	return(FALSE);
}

static gboolean ev_addrow_clicked(GtkButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;

	if (pbtwnd->byterows < MAXBYTEROWS)
		create_byterow(pbtwnd, pbtwnd->byterows);
	else {
		// TODO: yell at user
	}

	return(FALSE);
}

static gboolean ev_insert_clicked(GtkButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;
	int br_idx = (int)g_object_get_data(G_OBJECT(btn), Key_RowIdx);

	if (pbtwnd->byterows < MAXBYTEROWS)
		insert_byterow(pbtwnd, br_idx);
	else {
		// TODO: yell at user
	}

	return(FALSE);
}

static gboolean ev_remove_clicked(GtkButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;
	int br_idx = (int)g_object_get_data(G_OBJECT(btn), Key_RowIdx);
	remove_byterow(pbtwnd, br_idx);
	return(FALSE);
}

static gboolean ev_delay_clicked(GtkButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;
	int br_idx = (int)g_object_get_data(G_OBJECT(btn), Key_RowIdx);
	toggle_byterow_type(&pbtwnd->byterow[br_idx]);
	return(FALSE);
}

static gboolean ev_sendbyte_toggled(GtkToggleButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;
	int br_idx = (int)g_object_get_data(G_OBJECT(btn), Key_RowIdx);
	PByteRow pbr = &pbtwnd->byterow[br_idx];

	if (gtk_toggle_button_get_active(btn))
		update_hexvalue(pbr);
	else
		gtk_label_set_text(GTK_LABEL(pbr->lbl_hex), "");
	
	return(FALSE);
}

static gboolean ev_bitset_toggled(GtkToggleButton* btn, gpointer data)
{
	PBTWnd pbtwnd = (PBTWnd)data;
	int br_idx = (int)g_object_get_data(G_OBJECT(btn), Key_RowIdx);
	PByteRow pbr = &pbtwnd->byterow[br_idx];

	GtkToggleButton* tb = GTK_TOGGLE_BUTTON(pbr->cb_sendbyte);
	if (gtk_toggle_button_get_active(tb))
		update_hexvalue(pbr);
	else
		// will trigger an update
		gtk_toggle_button_set_active(tb, TRUE);

	return(FALSE);
}
