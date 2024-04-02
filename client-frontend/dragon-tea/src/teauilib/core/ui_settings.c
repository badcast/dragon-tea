#include "tea.h"

struct
{
    int toggle_log;
    int toggle_notify;
    int toggle_server;
} ref_setting;

void apply_changes()
{
    env.show_logs = ref_setting.toggle_log;
    env.old_notify_remove = ref_setting.toggle_notify;
    ref_setting.toggle_server =
        tea_get_server_id(env.servers[gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.settings_tab.combx_server_list))]);

    if(env.active_server != ref_setting.toggle_server)
    {
        tea_logout();
        tea_switch_server(ref_setting.toggle_server);
        tea_try_login();
    }
    tea_ui_focus_tab(UI_TAB_AUTH);

    // save
    tea_save();
}

void toggle_button(GtkWidget *widget, gpointer data)
{
    (*(int *) data) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void on_button_clk_show_log(GtkWidget *widget, gpointer data)
{
    show_log_dialog();
}

void tea_ui_update_settings()
{
    int selServer = 0;
    for(int x = 0; x < 32; ++x)
    {
        if(strlen(env.servers[x]) == 0)
            break;

        if(tea_get_server_id(env.servers[x]) == env.active_server)
        {
            selServer = x;
            break;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets.settings_tab.combx_server_list), (ref_setting.toggle_server = selServer));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.settings_tab.toggle_log), (ref_setting.toggle_log = env.show_logs));
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(widgets.settings_tab.toggle_notify), (ref_setting.toggle_notify = env.old_notify_remove));
}

GtkWidget *create_settings_widget()
{
    GtkWidget *widgetbox;

    widgetbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create List
    GtkListStore *server_list = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter iter;

    for(int x = 0; x < sizeof(env.servers[0]); ++x)
    {
        if(strlen(env.servers[x]) == 0)
            break;

        gtk_list_store_append(server_list, &iter);
        gtk_list_store_set(server_list, &iter, 0, env.servers[x], -1);
    }

    gtk_box_pack_start(GTK_BOX(widgetbox), gtk_label_new(_("Switch server: ")), TRUE, FALSE, 0);

    GtkWidget *combobox = widgets.settings_tab.combx_server_list = gtk_combo_box_new_with_model(GTK_TREE_MODEL(server_list));
    g_object_unref(server_list);

    gtk_widget_set_size_request(combobox, 200, 30);

    // Show contents
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
    gtk_box_pack_start(GTK_BOX(widgetbox), combobox, TRUE, FALSE, 0);

    GtkWidget *currentWidget = widgets.settings_tab.toggle_log = gtk_check_button_new_with_label(_("Show log window on startup"));
    g_signal_connect(G_OBJECT(currentWidget), "toggled", G_CALLBACK(toggle_button), &ref_setting.toggle_log);
    gtk_box_pack_start(GTK_BOX(widgetbox), currentWidget, TRUE, FALSE, 0);

    currentWidget = widgets.settings_tab.toggle_notify = gtk_check_button_new_with_label(_("Remove old notifications"));

    g_signal_connect(G_OBJECT(currentWidget), "clicked", G_CALLBACK(toggle_button), &ref_setting.toggle_notify);
    gtk_box_pack_start(GTK_BOX(widgetbox), currentWidget, TRUE, FALSE, 0);

    currentWidget = gtk_button_new_with_label("Show log");
    g_signal_connect(G_OBJECT(currentWidget), "clicked", G_CALLBACK(on_button_clk_show_log), NULL);
    gtk_box_pack_start(GTK_BOX(widgetbox), currentWidget, TRUE, FALSE, 0);

    GtkWidget *button_apply = widgets.settings_tab.button_apply = gtk_button_new_with_label(_("Apply changes"));
    g_signal_connect(button_apply, "clicked", G_CALLBACK(apply_changes), NULL);
    gtk_box_pack_end(GTK_BOX(widgetbox), button_apply, TRUE, FALSE, 0);

    tea_ui_update_settings();

    return widgetbox;
}
