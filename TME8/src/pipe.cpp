#include "pipe.h"

#include <fcntl.h>
#include <unistd.h>
#include <limits.h> // For PIPE_BUF
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <signal.h>
#include <algorithm>

namespace pr {

// Place this in shared memory !    
struct PipeShm {
    char buffer[PIPE_BUF];
    size_t head; // write position
    size_t tail; // read position
    size_t count; // number of bytes in the pipe
    size_t nbReaders; // number of readers
    size_t nbWriters; // number of writers
};

// This is per-process handle, not in shared memory
struct Pipe {
    PipeShm *shm; // pointer to shared memory
    int oflags; // O_RDONLY or O_WRONLY
    // TODO : semaphores
    public:
        sem_t *semMut;
        sem_t *canRead;
        sem_t *canWrite;
};

int pipe_create(const char *name) {
    // Construct shared memory name
    char shm_name[256];
    // add a '/' at the beginning for shm_open
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    
    // Try to create shared memory with O_CREAT|O_EXCL
    // Set size of shared memory
    // Map the shared memory 
    // Initialize the PipeShm structure
    // memset(shm, 0, sizeof(PipeShm));
    int fd = shm_open(shm_name, O_CREAT | O_EXCL, 0666);
    if (fd == -1) {
        perror("erreur sh_open");
        exit(1);
    }

    if (ftruncate(fd, sizeof(PipeShm)) == -1) {
        perror("erreur ftruncate");
        exit(1);
    }

    void *ptr = mmap(nullptr, sizeof(PipeShm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("erreur mmap");
        exit(1);
    }

    PipeShm *shm = new (ptr) PipeShm;
    
    memset(shm, 0 , sizeof(PipeShm));

    // Including semaphores
    sem_t *semMut;
    sem_t *canRead;
    sem_t *canWrite;

    if ((canRead = sem_open("/canReadPipe", O_CREAT | O_EXCL, 0666, 0)) == nullptr) {
        perror("erreur sem_open read");
        exit(1);
    }

    if ((canWrite = sem_open("/canWritePipe", O_CREAT | O_EXCL, 0666, 1)) == nullptr) {
        perror("erreur sem_open write");
        exit(1);
    }

    if ((semMut = sem_open("/mutexPipe", O_CREAT | O_EXCL, 0666, 1)) == nullptr) {
        perror("erreur sem_open mutex");
        exit(1);
    }

    // Unmap and close (setup persists in shared memory)
    munmap(shm, sizeof(PipeShm));
    sem_close(semMut);
    sem_close(canRead);
    sem_close(canWrite);
    close(fd);
    
    return 0;
}

Pipe * pipe_open(const char *name, int oflags) {
    // Construct shared memory name
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    
    // Open shared memory (without O_CREAT)
    // Map the shared memory
    // Can close fd after mmap
    // Increment nbReaders or nbWriters

    int fd = shm_open(shm_name, oflags);
    if (fd == -1) {
        perror("erreur shm_open");
        exit(1);
    }

    void *ptr = mmap(nullptr, sizeof(PipeShm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("erreur mmap");
        exit(1);
    }

    close(fd);

    PipeShm *shm = new (ptr) PipeShm;
    
    memset(shm, 0 , sizeof(PipeShm));
    
    // Create and return Pipe handle
    Pipe *handle = new Pipe();
    handle->shm = shm;
    handle->oflags = oflags;
    handle->canRead = sem_open("/canReadPipe", 0);
    if (handle->canRead == nullptr) {
        perror("erreur sem_open read");
        exit(1);
    }
    handle->canWrite = sem_open("/canWritePipe", 0);
    if (handle->canWrite == nullptr) {
        perror("erreur sem_open write");
        exit(1);
    }
    handle->semMut = sem_open("/mutexPipe", 0);
    if (handle->semMut == nullptr) {
        perror("erreur sem_open mutex");
        exit(1);
    }

    if (oflags == O_RDONLY) shm->nbReaders++;
    else if (oflags == O_WRONLY) shm->nbWriters++;

    return handle;
}

ssize_t pipe_read(Pipe *handle, void *buf, size_t count) {
    if (handle == nullptr || handle->oflags != O_RDONLY) {
        errno = EBADF;
        return -1;
    } else if (count > PIPE_BUF) {
        errno = EINVAL;
        return -1;
    }
    
    // wait until some data available or no writers
    if (handle->shm->count == 0 || handle->shm->nbWriters > 0) sem_wait(handle->canRead);

    // Check if pipe is empty and no writers : EOF
    if (handle->shm->count == 0 || handle->shm->nbWriters == 0) return 0;

    sem_wait(handle->semMut);

    // Read min(count, shm->count) bytes
    PipeShm *shm = handle->shm;
    size_t to_read = std::min(count, shm->count);
    char *output = (char *)buf;
    
    // Handle circular buffer: may need to copy in two parts
    size_t first_chunk = std::min(to_read, PIPE_BUF - shm->tail);
    memcpy(output, &shm->buffer[shm->tail], first_chunk);
    
    if (first_chunk < to_read) {
        // Wrap around to beginning of buffer
        memcpy(output + first_chunk, &shm->buffer[0], to_read - first_chunk);
    }
    
    shm->tail = (shm->tail + to_read) % PIPE_BUF;
    shm->count -= to_read;
    
    // warn other readers/writers if needed
    sem_post(handle->semMut);
    if (shm->count > 0) sem_post(handle->canRead);
    sem_post(handle->canWrite);
    
    return to_read;
}

ssize_t pipe_write(Pipe *handle, const void *buf, size_t count) {
    if (handle == nullptr || handle->oflags != O_WRONLY) {
        errno = EBADF;
        return -1;
    } else if (count > PIPE_BUF) {
        errno = EINVAL;
        return -1;
    }
    
    PipeShm *shm = handle->shm;
    
    // wait until *enough* space available or no readers
    while (count > PIPE_BUF - handle->shm->count) {
        sem_wait(handle->canWrite);
    }

    // Check if no readers => SIGPIPE
    if (handle->shm->nbReaders == 0) raise(SIGPIPE);
    
    sem_wait(handle->semMut);
    
    // Write count bytes
    const char *input = (const char *)buf;
    
    // Handle circular buffer: may need to copy in two parts
    size_t first_chunk = std::min(count, PIPE_BUF - shm->head);
    memcpy(&shm->buffer[shm->head], input, first_chunk);
    
    if (first_chunk < count) {
        // Wrap around to beginning of buffer
        memcpy(&shm->buffer[0], input + first_chunk, count - first_chunk);
    }
    
    shm->head = (shm->head + count) % PIPE_BUF;
    shm->count += count;

    // warn other readers/writers if needed
    sem_post(handle->semMut);   
    sem_post(handle->canRead);
    if (PIPE_BUF - handle->shm->count > 0) sem_post(handle->canWrite);
    
    return count;
}

int pipe_close(Pipe *handle) {
    if (handle == nullptr) {
        errno = EBADF;
        return -1;
    }
    
    PipeShm *shm = handle->shm;
    
    // Decrement reader or writer count
    // Warn other process as needed (e.g. if last reader/writer)

    if (handle->oflags == O_RDONLY) handle->shm->nbReaders--;
    else if (handle->oflags == O_WRONLY) handle->shm->nbWriters--;

    if(handle->shm->nbReaders == 0) sem_post(handle->canWrite);
    if(handle->shm->nbWriters == 0) sem_post(handle->canRead);
    
    // Unmap memory
    // Free handle
    munmap(shm, sizeof(PipeShm));
    sem_close(handle->semMut);
    sem_close(handle->canRead);
    sem_close(handle->canWrite);
    delete handle;
    
    return 0;
}

int pipe_unlink(const char *name) {
    // Construct shared memory name
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    
    // Unlink shared memory (this also destroys the embedded semaphores)
    sem_unlink("/canReadPipe");
    sem_unlink("/canWritePipe");
    sem_unlink("mutexPipe");
    
    return 0;
}

} // namespace pr

