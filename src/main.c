#include "tea.h"

int main(int argc, char *argv[])
{

    //Check runned on Root
    #if __unix__
        if(getuid() == 0)
        {
            printf("Program runned as root. It's maybe damage your system. Closing.");
            return EXIT_FAILURE;
        }
    #endif

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
