#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    const char* shm_name = "/shm_numaratoare";
    const char* sem_name = "/sem_numaratoare";

    // Cream memoria partajata (terenul neutru)
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int));
    int* mem = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    *mem = 1; // Initializam memoria cu 1

    // Cream semaforul si il setam liber (pe 1)
    sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 1);

    pid_t pid = fork(); // Cream al doilea proces
    srand(time(NULL) ^ getpid()); // Random diferit pt fiecare

    while (true) {
        sem_wait(sem); // REZERVAM SEMAFORUL
        
        if (*mem >= 1000) {
            sem_post(sem); 
            break; // Iesim daca am ajuns la 1000
        }

        int coin = rand() % 2 + 1; // Dam cu banul (1 sau 2)
        
        // Cat timp cade 2, scriem
        while (coin == 2 && *mem < 1000) {
            (*mem)++;
            cout << (pid == 0 ? "[Proces 2] " : "[Proces 1] ") 
                 << "Banul a picat 2! Scrie in memorie: " << *mem << endl;
            coin = rand() % 2 + 1; // Dam iar
        }

        sem_post(sem); // ELIBERAM SEMAFORUL
        usleep(500);   // Pauza mica sa poata intra si celalalt
    }

    if (pid != 0) {
        wait(NULL); // Asteptam sa termine amandoua
        shm_unlink(shm_name);
        sem_unlink(sem_name);
        cout << "\nGata! Ambele procese au ajuns la 1000 folosind memoria partajata." << endl;
    }

    return 0;
}
