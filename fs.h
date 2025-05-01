// // #pragma once
// // #include <stdint.h>
// // #include <stdio.h>
// // #include <time.h>
// // #include <string.h>
// // #include <stdbool.h>

// // #define BLOCK_SIZE 4096
// // #define MAX_BLOCKS 1024
// // #define MAX_INODES 256
// // #define MAX_FILENAME 256
// // #define JOURNAL_BLOCK 1
// // #define INODE_BLOCKS_START 2
// // #define INODE_BLOCKS_COUNT   ((MAX_INODES * sizeof(Inode) + BLOCK_SIZE - 1) / BLOCK_SIZE)
// // #define DATA_BLOCKS_START    (INODE_BLOCKS_START + INODE_BLOCKS_COUNT)  // Начало данных

// // typedef enum {
// //     FS_OK,
// //     FS_IO_ERROR,
// //     FS_NO_SPACE,
// //     FS_INVALID_INODE,
// //     FS_CORRUPTED_DATA,
// //     FS_INVALID_ARG
// // } FSError;

// // typedef enum {
// //     FILE_TYPE,
// //     DIR_TYPE
// // } InodeType;

// // typedef struct {
// //     uint32_t inode;
// //     char name[MAX_FILENAME];
// // } DirEntry;

// // typedef struct {
// //     uint32_t magic;
// //     uint32_t inode_count;
// //     uint32_t block_count;
// //     uint32_t journal_seq;  // Счётчик транзакций
// //     uint32_t inode_start;
// //     uint32_t bitmap_block;
// // } SuperBlock;

// // typedef struct {
// //     uint32_t id;
// //     InodeType type;
// //     uint32_t size;
// //     uint32_t blocks[12];  // 12 прямых блоков
// //     time_t ctime;
// //     bool is_used;
// //     uint32_t crc32;
// // } Inode;

// // typedef struct {
// //     FILE* disk;
// //     SuperBlock super;
// //     uint8_t* block_bitmap;  // Битовая карта блоков
// // } FileSystem;

// // // Прототипы функций
// // FSError fs_format(FileSystem* fs);
// // FSError fs_create(FileSystem* fs, InodeType type, Inode** out);
// // FSError fs_write(FileSystem* fs, Inode* inode, const void* data, size_t size);
// // FSError fs_read(FileSystem* fs, Inode* inode, void* buffer);
// // FSError fs_delete(FileSystem* fs, Inode* inode);
// // FSError dir_add_entry(FileSystem* fs, Inode* dir, const char* name, uint32_t inode_id);
// // FSError dir_lookup(FileSystem* fs, Inode* dir, const char* name, Inode** out);
// // FSError journal_log(FileSystem* fs, const char* op, uint32_t inode_id);

// #pragma once
// #include <stdint.h>
// #include <stdio.h>
// #include <time.h>
// #include <string.h>
// #include <stdbool.h>

// #define BLOCK_SIZE 4096
// #define MAX_BLOCKS 1024
// #define MAX_INODES 256
// #define MAX_FILENAME 256
// // #define JOURNAL_BLOCK 1
// // #define BITMAP_BLOCK (JOURNAL_BLOCK + 1) // Блок битовой карты
// // #define INODE_BLOCKS_START 3 // Начало inode после суперблока, журнала и битмапа

// #define BITMAP_BLOCKS (fs) (fs->super.jornal_block + 1)// Блок битовой карты
// #define INODE_BLOCKS_START (fs) (BITMAP_BLOCKS(fs) + 1) // Начало inode после суперблока, журнала и битмапа
// #define INODE_BLOCKS_COUNT ((MAX_INODES * sizeof(Inode) + BLOCK_SIZE - 1) / BLOCK_SIZE)
// #define DATA_BLOCKS_START (fs) (INODE_BLOCKS_START(fs) + INODE_BLOCKS_COUNT)

// typedef enum {
//     FS_OK,
//     FS_IO_ERROR,
//     FS_NO_SPACE,
//     FS_INVALID_INODE,
//     FS_CORRUPTED_DATA,
//     FS_INVALID_ARG
// } FSError;

// typedef enum {
//     FILE_TYPE,
//     DIR_TYPE
// } InodeType;

// typedef struct {
//     uint32_t inode;
//     char name[MAX_FILENAME];
// } DirEntry;

// // Структура JournalEntry должна быть объявлена до использования
// typedef struct {
//     uint32_t seq;
//     char op[16];         // Фиксированный буфер для названия операции
//     uint32_t inode;
//     time_t timestamp;
// } JournalEntry;

// typedef struct {
//     uint32_t magic;
//     uint32_t inode_count;
//     uint32_t block_count;
//     uint32_t journal_seq;
//     uint32_t journal_block;   
//     uint32_t inode_start;
//     uint32_t bitmap_block; 
// } SuperBlock;

// typedef struct {
//     uint32_t id;
//     InodeType type;
//     uint32_t size;
//     uint32_t blocks[12];  
//     time_t ctime;
//     bool is_used;
//     uint32_t crc32;
// } Inode;

// typedef struct {
//     FILE* disk;
//     SuperBlock super;
//     uint8_t* block_bitmap;  
// } FileSystem;

// // Прототипы функций
// FSError fs_format(FileSystem* fs);
// FSError fs_create(FileSystem* fs, InodeType type, Inode** out);
// FSError fs_write(FileSystem* fs, Inode* inode, const void* data, size_t size);
// FSError fs_read(FileSystem* fs, Inode* inode, void* buffer);
// FSError fs_delete(FileSystem* fs, Inode* inode);
// FSError dir_add_entry(FileSystem* fs, Inode* dir, const char* name, uint32_t inode_id);
// FSError dir_lookup(FileSystem* fs, Inode* dir, const char* name, Inode** out);
// FSError journal_log(FileSystem* fs, const char* op, uint32_t inode_id);
// FSError get_next_seq_number(FileSystem* fs, uint32_t* seq); // Добавлен прототип

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// Константы
#define BLOCK_SIZE 4096
#define MAX_BLOCKS 1024
#define MAX_INODES 256
#define MAX_FILENAME 256

// Уровни логирования
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// Коды ошибок
typedef enum {
    FS_OK,
    FS_IO_ERROR,
    FS_NO_SPACE,
    FS_INVALID_INODE,
    FS_CORRUPTED_DATA,
    FS_INVALID_ARG
} FSError;

// Типы inode
typedef enum {
    FILE_TYPE,
    DIR_TYPE
} InodeType;

// Запись в каталоге
typedef struct {
    uint32_t inode;
    char name[MAX_FILENAME];
} DirEntry;

// Запись в журнале
typedef struct {
    uint32_t seq;
    char op[16];
    uint32_t inode;
    time_t timestamp;
} JournalEntry;

// Суперблок
typedef struct {
    uint32_t magic;
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t journal_seq;
    uint32_t journal_block;  // Блок под журнал
    uint32_t inode_start;    // Начало таблицы inode
    uint32_t bitmap_block;   // Блок битовой карты
} SuperBlock;

// Inode
typedef struct {
    uint32_t id;
    InodeType type;
    uint32_t size;
    uint32_t blocks[12];
    time_t ctime;
    bool is_used;
    uint32_t crc32;
} Inode;

// Файловая система
typedef struct {
    FILE* disk;
    SuperBlock super;
    uint8_t* block_bitmap;
} FileSystem;

// Прототипы функций
FSError fs_init(FileSystem* fs, const char* disk_path);
void fs_close(FileSystem* fs);
FSError fs_format(FileSystem* fs);
FSError fs_create(FileSystem* fs, InodeType type, Inode** out);
FSError fs_write(FileSystem* fs, Inode* inode, const void* data, size_t size);
FSError fs_read(FileSystem* fs, Inode* inode, void* buffer);
FSError fs_delete(FileSystem* fs, Inode* inode);
FSError dir_add_entry(FileSystem* fs, Inode* dir, const char* name, uint32_t inode_id);
FSError dir_lookup(FileSystem* fs, Inode* dir, const char* name, Inode** out);
FSError journal_log(FileSystem* fs, const char* op, uint32_t inode_id);

// Прототип
FSError fs_find_inode(FileSystem* fs, uint32_t id, Inode* out);