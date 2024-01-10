#include "ui_callbacks.h"

gboolean on_refresh_traffic(gpointer);

void tea_ui_init()
{
    // Set Default Icon for ALL Owner Window (Dialog)
    GtkIconTheme *itheme = gtk_icon_theme_get_default();
    GtkIconInfo *iinfo = gtk_icon_theme_lookup_icon(itheme, "mail-message-new-symbolic", 32, 0);

    if(iinfo)
    {
        GdkPixbuf *icon_buf = gtk_icon_info_load_icon(iinfo, NULL);
        gtk_window_set_default_icon(icon_buf);
        g_object_unref(iinfo);
        g_object_unref(icon_buf);
    }

    // Init main window
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), _("Dragon Tea"));
    gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 400);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);

    widgets.main_window = GTK_WINDOW(main_window);
    g_signal_connect(main_window, "destroy", G_CALLBACK(ui_on_close_window), NULL);

    GtkWidget *notebook = widgets.notebook = gtk_notebook_new();
    GtkWidget *gboxContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(gboxContent), notebook, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), gboxContent);

    // Init widgets
    widgets.widget_auth = create_auth_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_auth, gtk_label_new(_("Authentication")));
    widgets.widget_main = create_chat_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_main, gtk_label_new(_("Dragon chat")));
    widgets.widget_settings = create_settings_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_settings, gtk_label_new(_("Configuration")));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook), gtk_image_new_from_icon_name("help-about", GTK_ICON_SIZE_LARGE_TOOLBAR), gtk_label_new(_("About")));
    g_signal_connect(notebook, "switch-page", G_CALLBACK(ui_on_notebook_switch_page), NULL);

    // Set margins
    for(int x = 0; x < gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)); ++x)
    {
        gtk_widget_set_margin_start(GTK_WIDGET(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), x)), 10);
        gtk_widget_set_margin_bottom(GTK_WIDGET(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), x)), 10);
        gtk_widget_set_margin_top(GTK_WIDGET(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), x)), 10);
        gtk_widget_set_margin_end(GTK_WIDGET(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), x)), 10);
    }

    // Статус
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *label = widgets.chat_tab.label_status = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(status_box), label, TRUE, TRUE, 0);
    label = widgets.chat_tab.label_traffics = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(status_box), label, FALSE, TRUE, 5);
    gtk_box_pack_end(GTK_BOX(status_box), gtk_image_new_from_icon_name("mail-send-receive", GTK_ICON_SIZE_SMALL_TOOLBAR), FALSE, TRUE, 0);

    gtk_box_pack_end(GTK_BOX(gboxContent), status_box, FALSE, TRUE, 0);

    g_timeout_add(1000, on_refresh_traffic, NULL);

    // Show window
    gtk_widget_show_all(main_window);

    tea_ui_auth_lock(FALSE);
    tea_ui_reg_lock(FALSE);

    // disable chat as default
    tea_ui_chat_enable(FALSE);

    // Probe auto sign in if the exists
    tea_try_login();
    tea_ui_focus_tab(UI_TAB_AUTH);

    show_log_dialog();
}

void tea_ui_update_settings()
{
    int selServer = 0;
    for(int x = 0; x < 32; ++x)
    {
        if(strlen(app_settings.servers[x]) == 0)
            break;

        if(tea_get_server_id(app_settings.servers[x]) == app_settings.active_server)
        {
            selServer = x;
            break;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets.settings_tab.combx_server_list), selServer);
}

void tea_ui_focus_tab(enum UiTabs tabIndex)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(widgets.notebook), (int) tabIndex);
}

void tea_ui_chat_enable(int value)
{
    tea_ui_chat_status_text("");
    tea_ui_chat_set_text_top("");
    gtk_entry_set_text(GTK_ENTRY(widgets.chat_tab.entry_message_set), "");

    tea_ui_chat_clear();

    if(!value)
        tea_ui_chat_set_text(_("You state is offline"));

    // interactable
    tea_ui_chat_interactable(value);
}

void tea_ui_chat_interactable(int value)
{
    gtk_widget_set_sensitive(widgets.widget_main, value);
    gtk_widget_set_sensitive(widgets.chat_tab.entry_message_set, value);
    gtk_widget_grab_focus(widgets.chat_tab.entry_message_set);
}

void tea_ui_chat_sync()
{
    widgets.chat_tab.chat_synched = FALSE;
    on_chat_message_handler_async(NULL);
}

void tea_ui_chat_status_text(const char *status_text)
{
    gtk_label_set_text(GTK_LABEL(widgets.chat_tab.label_status), status_text);
}

void tea_ui_chat_set_text_top(const char *text)
{
    gtk_label_set_text(GTK_LABEL(widgets.chat_tab.top_label_user_state), text);
}

void tea_ui_chat_set_text(const char *text)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    gtk_text_buffer_set_text(buffer, text, -1);
}

void tea_ui_chat_clear()
{
    tea_ui_chat_set_text("");
}

void tea_ui_chat_vscroll_max()
{
    // GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widgets.chat_tab.scrolled_window_chat));
    // gtk_adjustment_set_value(vadj, gtk_adjustment_get_upper(vadj));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    GtkTextIter endIter;
    gtk_text_buffer_get_end_iter(buffer, &endIter);
    GtkTextMark *end_mark = gtk_text_buffer_create_mark(buffer, "end", &endIter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat), end_mark, 0.0, TRUE, 0, 1.0);
    gtk_text_buffer_delete_mark(buffer, end_mark);
}

void tea_ui_chat_push_block(const struct tea_message_id *message)
{
    /*
     *  FORMAT VIEW for Message Block
     *   bad@(21:16): Hello
     */

    if(message == NULL)
    {
        printf("Message is null\n");
        return;
    }

    static const char format_message_block[] = "\n\n%s@(%s): %s\n";
    GDateTime *_gdatetime = g_date_time_new_from_unix_local(message->time_received);
    gchar *date_time = g_date_time_format(_gdatetime, "%H:%M");

    int require_size = snprintf(NULL, 0, format_message_block, message->sent_user_name, date_time, message->message_text);

    char *text_buffer = (char *) malloc(require_size + 1);
    if(text_buffer == NULL)
        ui_error_fail("Out of memory!");

    snprintf(text_buffer, require_size, format_message_block, message->sent_user_name, date_time, message->message_text);

    g_free(_gdatetime);
    g_free(date_time);

    tea_ui_chat_push_text_raw(text_buffer, -1);

    free(text_buffer);
}

void tea_ui_chat_push_text_raw(const char *text, int len)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text, len);
}

char *expower_bytesISO(char *str, int len, uintmax_t bytes)
{
    static const char data_kind[5][6] = {"bytes", "KiB", "MiB", "GiB", "TiB"};

    /*
    ASSIGN: bytes < 1024 = bytes
            bytes < 1024^2 = KiB
            bytes < 1024^3 = MiB
            bytes < 1024^4 = GiB
            bytes < 1024^5 = TiB
          ~ BigData
    */
    int power = 0;

    while(bytes >= 1024)
    {
        bytes /= 1024;
        ++power;
    }
    snprintf(str, len, "%ju %s", bytes, data_kind[MIN(power, 5)]);
    return str;
}

size_t last_sent = 0, last_recv = 0;
gboolean on_refresh_traffic(gpointer)
{
    char buffer[78], bufA[24];
    buffer[0] = bufA[0] = 0;

    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.transmitted_bytes - last_sent), sizeof(buffer));
    strncat(buffer, "/s / ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.received_bytes - last_recv), sizeof(buffer));
    strncat(buffer, "/s", sizeof(buffer));

    strncat(buffer, " | ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.transmitted_bytes), sizeof(buffer));
    strncat(buffer, " / ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.received_bytes), sizeof(buffer));

    last_sent = net_stats.transmitted_bytes;
    last_recv = net_stats.received_bytes;

    gtk_label_set_text(GTK_LABEL(widgets.chat_tab.label_traffics), buffer);

    return TRUE;
}
