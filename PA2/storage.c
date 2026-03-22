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
    // use File* and fopen because we need to use fpopen and fseek
    FILE* vault = fopen(".mgit/data.bin", "rb");
    fseek(vault, (long)offset, SEEK_SET)
    // TODO: Read `size` bytes and write them to `out_fd` using the write_all() helper.
    char buffer[4096];
    uint32_t bytes_remaining = size;

    while (bytes_remaining > 0) {
        uint32_t to_read = bytes_remaining < sizeof(buffer) ? bytes_remaining : sizeof(buffer);
        size_t bytes_read = fread(buffer, 1, to_read, vault);
        if (bytes_read == 0) break;

        write_all(out_fd, buffer, bytes_read);
        bytes_remaining -= bytes_read;
    }
    fclose(vault);
}


// --- Snapshot Management ---
void store_snapshot_to_disk(Snapshot* snap)
{
    // Build the filename e.g. ".mgit/snapshots/snap_003.bin"
    char path[64];
    snprintf(path, sizeof(path), ".mgit/snapshots/snap_%03u.bin", snap->id);

    FILE* fd = fopen(path, "wb");

    // Write snapshot header
    fwrite(&snap->id,sizeof(uint32_t), 1, fd);
    fwrite(&snap->timestamp, sizeof(uint64_t), 1, fd);

    uint32_t msg_len = strlen(snap->message);
    fwrite(&msg_len,sizeof(uint32_t), 1,fd);
    fwrite(snap->message, sizeof(char),msg_len, fd);

    // Count files in linked list and write the count first
    uint32_t num_files = 0;
    FileEntry* cur = snap->files;
    while (cur) { num_files++; cur = cur->next; }
    fwrite(&num_files, sizeof(uint32_t), 1, fp);

    // Write each FileEntry + its BlockTable
    cur = snap->files;
    while (cur) {
        uint32_t name_len = strlen(cur->filename);
        fwrite(&name_len,sizeof(uint32_t), 1, fp);
        fwrite(cur->filename, sizeof(char),name_len, fp);

        fwrite(&cur->block->physical_offset, sizeof(uint64_t), 1, fp);
        fwrite(&cur->block->size, sizeof(uint32_t), 1, fp);

        cur = cur->next;
    }

    fclose(fp);
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
