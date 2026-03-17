#include "mgit.h"
#include <errno.h>
#include <zstd.h>

// --- Helper Functions ---
uint32_t get_current_head()
{
    int fd = open(".mgit/HEAD", O_RDONLY);

    char buf[32];
    int n = read(fd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        perror("read HEAD");
        close(fd);
        return 0;
    }

    buf[n] = '\0';  // null terminator 
    close(fd);
    return atoi(buf);
}
void update_head(uint32_t new_id)
{
    int fd = open(".mgit/HEAD", O_WRONLY | O_TRUNC);
    
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%u\n", new_id);
    write(fd, buf, len);
    close(fd);
}

// --- Blob Storage (Raw) ---
void write_blob_to_vault(const char* filepath, BlockTable* block)
{
    
    // TODO: Open `filepath` for reading (rb).
    int fd = open(filepath, O_RDONLY);
    // TODO: Open `.mgit/data.bin` for APPENDING (ab).
    int vault_fd = open(".mgit/data.bin", O_WRONLY | O_APPEND);
    // TODO: Use ftell() to record the current end of the vault into block->physical_offset.
    long position = ftell(vault_fd);
    block->physical_offset = position;

    // TODO: Read the file bytes and write them into the vault. Update block->size.
    char buffer[4096];
    ssize_t bytes_read;
    block->size = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, vault_fp);
        block->size += bytes_read;
    }

    close(fd);
    fclose(vault_fp);
}

void read_blob_from_vault(uint64_t offset, uint32_t size, int out_fd)
{

    // TODO: Open the vault, fseek() to the physical_offset.
    FILE* vault = fopen(".mgit/data.bin", "rb");
    fseek(vault, (long)offset, SEEK_SET)
    // TODO: Read `size` bytes and write them to `out_fd` using the write_all() helper.


}

// --- Snapshot Management ---
void store_snapshot_to_disk(Snapshot* snap)
{
    // TODO: Serialize the Snapshot struct and its linked list of FileEntry/BlockTables
    // into a binary file inside `.mgit/snapshots/snap_XXX.bin`.
}

Snapshot* load_snapshot_from_disk(uint32_t id)
{
    // TODO: Read a `snap_XXX.bin` file and reconstruct the Snapshot struct
    // and its FileEntry linked list in heap memory.
    return NULL;
}

void chunks_recycle(uint32_t target_id)
{
    // TODO: Garbage Collection (The Vacuum)
    // 1. Load the oldest snapshot (target_id) and the newest snapshot (HEAD).
    // 2. Iterate through the oldest snapshot's files.
    // 3. If a chunk's physical_offset is NOT being used by ANY file in the HEAD snapshot,
    //    it is "stalled". Zero out those specific bytes in `data.bin`.
}

void mgit_snapshot(const char* msg)
{
    // TODO: 1. Get current HEAD ID and calculate next_id. Load previous files for crawling.
    // TODO: 2. Call build_file_list_bfs() to get the new directory state.

    // TODO: 3. Iterate through the new file list.
    // - If a file has data (chunks) but its size is 0, it needs to be written to the vault.
    // - CRITICAL: Check for Hard Links! If another file in the *current* list with the same
    //   inode was already written to the vault, copy its offset and size. DO NOT write twice!
    // - Call write_blob_to_vault() for new files.

    // TODO: 4. Call store_snapshot_to_disk() and update_head().
    // TODO: 5. Free memory.
    // TODO: 6. Enforce MAX_SNAPSHOT_HISTORY (5). If exceeded, call chunks_recycle()
    //          and delete the oldest manifest file using remove().
}
