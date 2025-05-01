#include "fs.h"

static FSError allocate_block(FileSystem* fs, uint32_t* block_id) {
    for (uint32_t i = fs->super.inode_start + MAX_INODES; i < MAX_BLOCKS; i++) {
        int byte = i / 8;
        int bit = i % 8;
        if (!(fs->block_bitmap[byte] & (1 << bit))) {
            fs->block_bitmap[byte] |= (1 << bit);
            *block_id = i;
            return FS_OK;
        }
    }
    return FS_NO_SPACE;
}

FSError fs_write(FileSystem* fs, Inode* inode, const void* data, size_t size) {
    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocks_needed > 12) return FS_NO_SPACE;

    // Журналирование
    FSError err = journal_log(fs, "WRITE", inode->id);
    if (err != FS_OK) return err;

    const uint8_t* bytes = (const uint8_t*)data;
    for (int i = 0; i < blocks_needed; i++) {
        uint32_t block_id;
        if (allocate_block(fs, &block_id) != FS_OK) return FS_NO_SPACE;

        if (fseek(fs->disk, block_id * BLOCK_SIZE, SEEK_SET) != 0) 
            return FS_IO_ERROR;

        size_t chunk = (i == blocks_needed - 1) ? size % BLOCK_SIZE : BLOCK_SIZE;
        if (fwrite(bytes + i * BLOCK_SIZE, 1, chunk, fs->disk) != chunk) 
            return FS_IO_ERROR;

        inode->blocks[i] = block_id;
    }

    inode->size = size;
    return FS_OK;
}