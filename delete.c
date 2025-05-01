#include "fs.h"

FSError fs_delete(FileSystem* fs, Inode* inode) {
    if (!inode || !inode->is_used) return FS_INVALID_INODE;

    // Журналирование
    FSError err = journal_log(fs, "DELETE", inode->id);
    if (err != FS_OK) return err;

    // Освобождение блоков
    for (int i = 0; i < 12; i++) {
        if (inode->blocks[i] != 0) {
            int byte = inode->blocks[i] / 8;
            int bit = inode->blocks[i] % 8;
            fs->block_bitmap[byte] &= ~(1 << bit);
        }
    }

    // Пометить inode как свободный
    inode->is_used = false;
    uint32_t offset = fs->super.inode_start * BLOCK_SIZE + inode->id * sizeof(Inode);
    if (fseek(fs->disk, offset, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(inode, sizeof(Inode), 1, fs->disk) != 1) return FS_IO_ERROR;

    return FS_OK;
}