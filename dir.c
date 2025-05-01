#include "fs.h"

FSError dir_add_entry(FileSystem* fs, Inode* dir, const char* name, uint32_t inode_id) {
    DirEntry entry = {.inode = inode_id};
    strncpy(entry.name, name, MAX_FILENAME - 1);
    entry.name[MAX_FILENAME - 1] = '\0';

    return fs_write(fs, dir, &entry, sizeof(DirEntry));
}

FSError dir_lookup(FileSystem* fs, Inode* dir, const char* name, Inode** out) {
    uint8_t buffer[dir->size];
    if (fs_read(fs, dir, buffer) != FS_OK) return FS_IO_ERROR;

    DirEntry* entries = (DirEntry*)buffer;
    for (size_t i = 0; i < dir->size / sizeof(DirEntry); i++) {
        if (strcmp(entries[i].name, name) == 0) {
            uint32_t offset = fs->super.inode_start * BLOCK_SIZE + entries[i].inode * sizeof(Inode);
            if (fseek(fs->disk, offset, SEEK_SET) != 0) return FS_IO_ERROR;
            if (fread(*out, sizeof(Inode), 1, fs->disk) != 1) return FS_IO_ERROR;
            return FS_OK;
        }
    }
    return FS_INVALID_INODE;
}