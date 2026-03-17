#include "mgit.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    // Basic routing logic is provided.
    if (argc < 2)
        return 1;

    if (strcmp(argv[1], "init") == 0) {
        mgit_init();
    } else if (strcmp(argv[1], "snapshot") == 0) {
        if (argc < 3)
            return 1;
        mgit_snapshot(argv[2]);
    } else if (strcmp(argv[1], "send") == 0) {
        mgit_send(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(argv[1], "receive") == 0) {
        if (argc < 3)
            return 1;
        mgit_receive(argv[2]);
    } else if (strcmp(argv[1], "show") == 0) {
        mgit_show(argc > 2 ? argv[2] : NULL);
    } else if (strcmp(argv[1], "restore") == 0) {
        if (argc < 3)
            return 1;
        mgit_restore(argv[2]);
    }
    return 0;
}

void mgit_init()
{
    // TODO: Safely initialize the repository structure.
    struct stat st;

    // HINT: Check if ".mgit" already exists using stat(). If it does, do NOTHING
    // to prevent accidental data destruction.
    if (stat(".mgit", &st) == 0) {
        return;
    }



    // TODO: Create the following directories with 0755 permissions:
    // 1. ".mgit"
    // 2. ".mgit/snapshots"
    if (mkdir(".mgit", 0755) < 0) {
        if (errno != EEXIST) {
            perror("Failed to create .mgit directory");
            return;
        }
    }

    if (mkdir(".mgit/snapshots", 0755) < 0) {
        if (errno != EEXIST) {
            perror("Failed to create .mgit/snapshots directory");
            return;
        }
    }

    // TODO: Create the vault file ".mgit/data.bin".
    // HINT: Open with O_CREAT | O_WRONLY and 0644 permissions. Do NOT use O_TRUNC!
    int fd = open(".mgit/data.bin", O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        perror("Failed to create .mgit/data.bin");
        close(fd);
        return;
    }
    close(fd);
    // TODO: Create ".mgit/HEAD" and write "0" into it to initialize the snapshot counter.
    int head_fd = open(".mgit/HEAD", O_CREAT | O_WRONLY, 0644);
    if (head_fd < 0) {
        perror("Failed to create .mgit/HEAD");
        close(fd);
        return;
    }   
    write(fd, "0", 1);
    fclose(fd);
}
