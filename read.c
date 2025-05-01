#include "fs.h"

FSError fs_read(FileSystem* fs, Inode* inode, void* buffer) {
    if (!inode || !inode->is_used) return FS_INVALID_INODE;

    uint8_t* bytes = (uint8_t*)buffer;
    size_t remaining = inode->size;

    for (int i = 0; i < 12 && remaining > 0; i++) {
        uint32_t block_id = inode->blocks[i];
        if (block_id == 0) break;

        if (fseek(fs->disk, block_id * BLOCK_SIZE, SEEK_SET) != 0) 
            return FS_IO_ERROR;

        size_t chunk = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        if (fread(bytes, 1, chunk, fs->disk) != chunk) 
            return FS_IO_ERROR;

        bytes += chunk;
        remaining -= chunk;
    }

    // Проверка CRC32
    // if (inode->crc32 != crc32(buffer, inode->size)) 
    //     return FS_CORRUPTED_DATA;

    return FS_OK;
}