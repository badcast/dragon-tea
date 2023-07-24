#include "tea.h"

void show_about_dialog(gpointer,gpointer)
{
    // Создание GtkAboutDialog
    GtkAboutDialog *about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about_dialog, "Драконий Чай");
    gtk_about_dialog_set_version(about_dialog, "1.0");
    gtk_about_dialog_set_comments(about_dialog, "Свободный мессенджер для обмена информациями.");
    gtk_about_dialog_set_website(about_dialog, "https://github.com/badcast/dragon-tie");
    gtk_about_dialog_set_website_label(about_dialog, "Посетить сайт");
    gtk_about_dialog_set_authors(about_dialog, (const gchar *const[]) {"badcast", NULL});
    gtk_about_dialog_set_license(about_dialog, "GPL3");

    // Добавление кнопки "Закрыть"
    gtk_dialog_add_button(GTK_DIALOG(about_dialog), "Закрыть", GTK_RESPONSE_CLOSE);

    // Отображение диалогового окна "О программе"
    gtk_widget_show_all(GTK_WIDGET(about_dialog));

    // Ожидание закрытия диалогового окна
    gint response = gtk_dialog_run(GTK_DIALOG(about_dialog));
    if(response == GTK_RESPONSE_CLOSE)
    {
        gtk_widget_destroy(GTK_WIDGET(about_dialog));
    }
}
