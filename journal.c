#include "fs.h"
#include <time.h>

FSError get_next_seq_number(FileSystem* fs, uint32_t* seq) {
    if (fseek(fs->disk, 0, SEEK_SET) != 0) return FS_IO_ERROR;

    SuperBlock sb;
    if (fread(&sb, sizeof(SuperBlock), 1, fs->disk) != 1) return FS_IO_ERROR;
    *seq = sb.journal_seq + 1;
    sb.journal_seq = *seq;

    if (fseek(fs->disk, 0, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(&sb, sizeof(SuperBlock), 1, fs->disk) != 1) return FS_IO_ERROR;

    return FS_OK;
}

FSError journal_log(FileSystem* fs, const char* op, uint32_t inode_id) {
    JournalEntry entry;
    uint32_t seq;
    FSError err = get_next_seq_number(fs, &seq);
    if (err != FS_OK) return err;

    strncpy(entry.op, op, sizeof(entry.op) - 1);
    entry.op[sizeof(entry.op) - 1] = '\0';
    entry.seq = seq;
    entry.inode = inode_id;
    entry.timestamp = time(NULL);

    // Запись в журнал (блок 1)
    if (fseek(fs->disk, fs->super.journal_block * BLOCK_SIZE, SEEK_SET) != 0) 
        return FS_IO_ERROR;

    if (fwrite(&entry, sizeof(JournalEntry), 1, fs->disk) != 1) 
        return FS_IO_ERROR;

    return FS_OK;
}