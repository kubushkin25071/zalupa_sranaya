#include "fs.h"

FSError fs_create(FileSystem* fs, InodeType type, Inode** out) {
    for (uint32_t i = 1; i < fs->super.inode_count; i++) {
        Inode inode;
        uint32_t offset = fs->super.inode_start * BLOCK_SIZE + i * sizeof(Inode);

        if (fseek(fs->disk, offset, SEEK_SET) != 0) return FS_IO_ERROR;
        if (fread(&inode, sizeof(Inode), 1, fs->disk) != 1) return FS_IO_ERROR;

        if (!inode.is_used) {
            inode.id = i;
            inode.type = type;
            inode.is_used = true;
            inode.ctime = time(NULL);
            inode.size = 0;
            memset(inode.blocks, 0, sizeof(inode.blocks));

            if (fwrite(&inode, sizeof(Inode), 1, fs->disk) != 1) return FS_IO_ERROR;
            *out = &inode;
            return FS_OK;
        }
    }
    return FS_NO_SPACE;
}