#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "serial.h"

#include <gtk/gtk.h>

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);
    PBTWnd pbtwnd = create_mw();
    if (!pbtwnd)
        return(-1);
    gtk_widget_show(pbtwnd->mw);
    gtk_main();
    free(pbtwnd);
    return (0);
}
