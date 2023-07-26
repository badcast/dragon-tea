#include "tea.h"

int main(int argc, char *argv[])
{
    // Glib init
    g_thread_init(NULL);

    // Init networking
    net_init();

    // Инициализация GTK
    gtk_init(&argc, &argv);

    // Запускаем Чай
    tea_init();

    // Запуск основного цикла GTK
    gtk_main();

    net_free();

    return 0;
}
