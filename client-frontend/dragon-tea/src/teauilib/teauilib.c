#include <fcntl.h>

#include "tea_main.h"

#include "tea.h"

#define USE_ONCE_LAUNCH 0

#if USE_ONCE_LAUNCH
#include <semaphore.h>
#define TSEM_KNAME "dragon_tea_lock"
#endif

int TEA_API tea_main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    bindtextdomain(DRAGON_TEA_TEXTDOMAIN, DRAGON_TEA_LOCALE_DIR);
    textdomain(DRAGON_TEA_TEXTDOMAIN);

// Check runned on Root
#ifdef TEA_OS_LINUX
    if(getuid() == 0)
    {
        printf(_("The program is running as root. These privileges can harm the system, please be aware of this. Closing."));
        return EXIT_FAILURE;
    }
#endif

    // GUI Init
    gtk_init(&argc, &argv);

#if USE_ONCE_LAUNCH
    // Check is only one copy runs
    sem_t *sem;

    sem = sem_open(TSEM_KNAME, O_CREAT | O_EXCL, 0644, 1);
    if(sem == SEM_FAILED)
    {
        ui_error("Program already launched.");
        return EXIT_FAILURE;
    }

    if(sem_wait(sem) == -1)
    {
        perror("sem_wait");
        return EXIT_FAILURE;
    }
#endif

    // Run Dragon Tea Messenger
    tea_init();

    // Window Looping
    gtk_main();

    // Free Unused resources DTM
    tea_free();

#if USE_ONCE_LAUNCH
    // Close launch
    if(sem_post(sem) == -1)
    {
        perror("sem_post");
        return EXIT_FAILURE;
    }
    if(sem_close(sem) == -1)
    {
        perror("sem_close");
        return EXIT_FAILURE;
    }

    if(sem_unlink(TSEM_KNAME) == -1)
    {
        perror("sem_unlink");
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}
