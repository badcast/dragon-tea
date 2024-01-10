#include "tea.h"

void apply_changes()
{
    tea_logout();

    int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.settings_tab.combx_server_list));

    // Switch server
    tea_switch_server(tea_get_server_id(app_settings.servers[selected]));

    tea_ui_focus_tab(UI_TAB_AUTH);
    tea_try_login();
}

GtkWidget *create_settings_widget()
{
    GtkWidget *widgetbox;

    widgetbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create List
    GtkListStore *server_list = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter iter;

    for(int x = 0; x < sizeof(app_settings.servers[0]); ++x)
    {
        if(strlen(app_settings.servers[x]) == 0)
            break;

        gtk_list_store_append(server_list, &iter);
        gtk_list_store_set(server_list, &iter, 0, app_settings.servers[x], -1);
    }

    gtk_box_pack_start(GTK_BOX(widgetbox), gtk_label_new(_("Switch server: ")), TRUE, FALSE, 0);

    GtkWidget *combobox = widgets.settings_tab.combx_server_list = gtk_combo_box_new_with_model(GTK_TREE_MODEL(server_list));
    g_object_unref(server_list);

    gtk_widget_set_size_request(combobox, 200, 30);

    // Создание отображения текста
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);

    gtk_box_pack_start(GTK_BOX(widgetbox), combobox, TRUE, FALSE, 0);

    GtkWidget *button_apply = widgets.settings_tab.button_apply = gtk_button_new_with_label(_("Apply changes"));
    g_signal_connect(button_apply, "clicked", G_CALLBACK(apply_changes), NULL);

    gtk_box_pack_end(GTK_BOX(widgetbox), button_apply, TRUE, FALSE, 0);

    tea_ui_update_settings();

    return widgetbox;
}
