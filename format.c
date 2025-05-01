#include "fs.h"
#include <stdlib.h>

FSError fs_format(FileSystem* fs) {
    if (fs->disk == NULL) return FS_IO_ERROR;

    // Инициализация суперблока
    SuperBlock sb = {
        .magic = 0xED05F5,
        .inode_count = MAX_INODES-1,
        .block_count = MAX_BLOCKS,
        .journal_seq = 0,
        .journal_block = 1,      // Журнал в блоке 1
        .bitmap_block = 2,       // Битовая карта в блоке 2
        .inode_start = 3         // Inode начинаются с блока 3
    };

    // Запись суперблока в блок 0
    if (fseek(fs->disk, 0, SEEK_SET) != 0) return FS_IO_ERROR;
    if (fwrite(&sb, sizeof(SuperBlock), 1, fs->disk) != 1) return FS_IO_ERROR;

    // Инициализация битовой карты (блоки 0, 1, 2 заняты)
    fs->block_bitmap = calloc(MAX_BLOCKS / 8, 1);
    if (!fs->block_bitmap) return FS_NO_SPACE;
    fs->block_bitmap[0] = 0x07;

    // Запись битовой карты в блок 2
    if (fseek(fs->disk, sb.bitmap_block * BLOCK_SIZE, SEEK_SET) != 0) {
        free(fs->block_bitmap);
        return FS_IO_ERROR;
    }
    if (fwrite(fs->block_bitmap, MAX_BLOCKS / 8, 1, fs->disk) != 1) {
        free(fs->block_bitmap);
        return FS_IO_ERROR;
    }

    // Создание корневого каталога
    Inode root = {
        .id = 0,
        .type = DIR_TYPE,
        .ctime = time(NULL),
        .is_used = true
    };

    // Запись корневого каталога в блок 3
    if (fseek(fs->disk, sb.inode_start * BLOCK_SIZE, SEEK_SET) != 0) {
        free(fs->block_bitmap);
        return FS_IO_ERROR;
    }
    if (fwrite(&root, sizeof(Inode), 1, fs->disk) != 1) {
        free(fs->block_bitmap);
        return FS_IO_ERROR;
    }

    return FS_OK;
}
