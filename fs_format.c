#include "fs.h"
#include <stdlib.h>

FSError fs_format(FileSystem* fs) {
    // Инициализация суперблока
    SuperBlock sb = {
        .magic = 0xED05F5,
        .inode_count = MAX_INODES,
        .block_count = MAX_BLOCKS,
        .journal_seq = 0,
        .inode_start = INODE_BLOCKS_START,
        .bitmap_block = JOURNAL_BLOCK + 1
    };

    // Запись суперблока в блок 0
    if (fseek(fs->disk, 0, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(&sb, sizeof(SuperBlock), 1, fs->disk) != 1) return FS_IO_ERROR;

    // Инициализация битовой карты (все блоки свободны, кроме 0, 1 и 2)
    fs->block_bitmap = calloc(MAX_BLOCKS / 8, 1);
    fs->block_bitmap[0] = 0x07;  // Блоки 0, 1, 2 заняты

    if (fseek(fs->disk, JOURNAL_BLOCK * BLOCK_SIZE, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(fs->block_bitmap, MAX_BLOCKS / 8, 1, fs->disk) != 1) return FS_IO_ERROR;

    // Создание корневого каталога
    Inode root = {
        .id = 0,
        .type = DIR_TYPE,
        .is_used = true,
        .ctime = time(NULL)
    };

    if (fseek(fs->disk, INODE_BLOCKS_START * BLOCK_SIZE, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(&root, sizeof(Inode), 1, fs->disk) != 1) return FS_IO_ERROR;

    return FS_OK;
}
