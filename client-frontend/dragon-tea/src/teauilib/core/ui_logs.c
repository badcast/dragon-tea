#include "tea.h"

void tea_log(const char *message)
{
    int msgLen;
    if(message == NULL || ((msgLen = strlen(message)) == 0))
    {
        printf("Log is null");
        return;
    }

    printf("%s\n", message);

    GtkTextIter eIter;
    gtk_text_buffer_get_end_iter(env.log_buffer, &eIter);
    gtk_text_buffer_insert(env.log_buffer, &eIter, message, -1);

    gtk_text_buffer_get_end_iter(env.log_buffer, &eIter);
    gtk_text_buffer_insert(env.log_buffer, &eIter, "\n", -1);
}

void tea_clear_log()
{
    gtk_text_buffer_set_text(env.log_buffer, "", 0);
}

void tea_ui_clear_buffer(GtkWidget *, gpointer)
{
    tea_clear_log();
}

void show_log_dialog()
{
    gint x, y, w;
    gtk_window_get_position(widgets.main_window, &x, &y);
    gtk_window_get_size(widgets.main_window, &w, NULL);

    widgets.log_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_default_size(widgets.log_window, 300, 400);
    gtk_window_move(widgets.log_window, x + w, y);
    GtkBox *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *scrolled_text = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_text), GTK_SHADOW_IN);
    gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(scrolled_text), TRUE);

    GtkWidget *textView = gtk_text_view_new_with_buffer(env.log_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);

    gtk_container_add(GTK_CONTAINER(scrolled_text), textView);

    gtk_box_pack_start(box, scrolled_text, TRUE, TRUE, 0);

    GtkWidget *clearButton = gtk_button_new_with_mnemonic(_("Clear log"));
    g_signal_connect(G_OBJECT(clearButton), "clicked", G_CALLBACK(tea_ui_clear_buffer), NULL);

    gtk_box_pack_end(box, clearButton, FALSE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(widgets.log_window), box);
    gtk_widget_show_all(GTK_WIDGET(widgets.log_window));
}
