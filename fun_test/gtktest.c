#include <cairo/cairo.h>
#include <gtk/gtk.h>

static void do_drawing(cairo_t *);


static void tran_setup(GtkWidget *win)
{        
  GdkScreen *screen;
  GdkVisual *visual;
  
  gtk_widget_set_app_paintable(win, TRUE);
  screen = gdk_screen_get_default();
  visual = gdk_screen_get_rgba_visual(screen);
  
  if (visual != NULL && gdk_screen_is_composited(screen)) {
      gtk_widget_set_visual(win, visual);
  }
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{      
  do_drawing(cr);  

  return FALSE;
}

static void do_drawing(cairo_t *cr)
{
  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.4);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
}

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  tran_setup(window);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 250); 
  gtk_window_set_title(GTK_WINDOW(window), "Transparent window");

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}