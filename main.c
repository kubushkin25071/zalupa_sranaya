#include "fs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Вспомогательные функции
static void setup(FileSystem* fs) {
    fs->disk = fopen("test_fs.img", "w+b");
    assert(fs->disk != NULL);
    assert(fs_format(fs) == FS_OK);
}

static void teardown(FileSystem* fs) {
    fclose(fs->disk);
    remove("test_fs.img");
}

FSError fs_find_inode(FileSystem* fs, uint32_t id, Inode* out) {
    if (id >= fs->super.inode_count) 
        return FS_INVALID_INODE;

    uint32_t offset = fs->super.inode_start * BLOCK_SIZE + id * sizeof(Inode);
    if (fseek(fs->disk, offset, SEEK_SET) != 0) 
        return FS_IO_ERROR;

    if (fread(out, sizeof(Inode), 1, fs->disk) != 1) 
        return FS_IO_ERROR;

    if (!out->is_used) 
        return FS_INVALID_INODE;

    return FS_OK;
}

// Тест 1: Базовые операции (создание/удаление/чтение/запись)
void test_basic_operations() {
    FileSystem fs;
    setup(&fs);

    // Создание файла
    Inode* file;
    assert(fs_create(&fs, FILE_TYPE, &file) == FS_OK);
    assert(file != NULL);

    // Запись данных
    const char* data = "Test data for basic operations";
    assert(fs_write(&fs, file, data, strlen(data) + 1) == FS_OK);

    // Чтение данных
    char buffer[128];
    assert(fs_read(&fs, file, buffer) == FS_OK);
    assert(strcmp(data, buffer) == 0);

    // Удаление файла
    assert(fs_delete(&fs, file) == FS_OK);

    teardown(&fs);
}

// Тест 2: Работа с каталогами
void test_directories() {
    FileSystem fs;
    setup(&fs);

    // Создание корневого каталога
    Inode* root;
    assert(fs_create(&fs, DIR_TYPE, &root) == FS_OK);

    // Создание файлов и добавление их в каталог
    Inode* files[3];
    const char* filenames[] = {"file1.txt", "file2.txt", "file3.txt"};
    
    for (int i = 0; i < 3; i++) {
        assert(fs_create(&fs, FILE_TYPE, &files[i]) == FS_OK);
        assert(dir_add_entry(&fs, root, filenames[i], files[i]->id) == FS_OK);
    }

    // Поиск файлов в каталоге
    for (int i = 0; i < 3; i++) {
        Inode* found;
        assert(dir_lookup(&fs, root, filenames[i], &found) == FS_OK);
        assert(found->id == files[i]->id);
    }

    teardown(&fs);
}

// Тест 3: Работа с несколькими файлами
void test_multiple_files() {
    FileSystem fs;
    setup(&fs);

    // Создание 5 файлов
    Inode* files[5];
    const char* data = "Sample content";
    
    for (int i = 0; i < 5; i++) {
        assert(fs_create(&fs, FILE_TYPE, &files[i]) == FS_OK);
        assert(fs_write(&fs, files[i], data, strlen(data) + 1) == FS_OK);
    }

    // Проверка содержимого
    for (int i = 0; i < 5; i++) {
        char buffer[64];
        assert(fs_read(&fs, files[i], buffer) == FS_OK);
        assert(strcmp(data, buffer) == 0);
    }

    teardown(&fs);
}

// Тест 4: Проверка целостности данных (CRC32)
void test_data_integrity() {
    FileSystem fs;
    setup(&fs);

    Inode* file;
    const char* data = "Data with CRC check";
    assert(fs_create(&fs, FILE_TYPE, &file) == FS_OK);
    assert(fs_write(&fs, file, data, strlen(data) + 1) == FS_OK);

    // Чтение inode через fs_find_inode
    Inode check_inode;
    assert(fs_find_inode(&fs, file->id, &check_inode) == FS_OK);

    // Порча данных
    uint32_t corrupted_block = check_inode.blocks[0];
    fseek(fs.disk, corrupted_block * BLOCK_SIZE, SEEK_SET);
    fwrite("CORRUPTED", 1, 9, fs.disk);

    // Попытка чтения
    char buffer[64];
    assert(fs_read(&fs, &check_inode, buffer) == FS_CORRUPTED_DATA);

    teardown(&fs);
}

// Тест 5: Восстановление журнала
void test_journal_recovery() {
    FileSystem fs;
    setup(&fs);

    // Создаем файл и пишем в журнал
    Inode* file;
    assert(fs_create(&fs, FILE_TYPE, &file) == FS_OK);
    assert(journal_log(&fs, "CREATE", file->id) == FS_OK);

    // Проверяем, что запись в журнале существует
    JournalEntry entry;
    if (fseek(fs.disk, fs.super.journal_block * BLOCK_SIZE, SEEK_SET) != 0) {
        assert(0 && "Ошибка чтения журнала");
    }
    assert(fread(&entry, sizeof(JournalEntry), 1, fs.disk) == 1);
    assert(entry.inode == file->id);

    teardown(&fs);
}

// Тест 6: Обработка ошибок (нехватка места)
void test_no_space() {
    FileSystem fs;
    setup(&fs);

    // Заполнение всех inode
    Inode* dummy;
    for (int i = 0; i < MAX_INODES; i++) {
        if (fs_create(&fs, FILE_TYPE, &dummy) != FS_OK) break;
    }

    // Попытка создать ещё один файл
    Inode* file;
    assert(fs_create(&fs, FILE_TYPE, &file) == FS_NO_SPACE);

    teardown(&fs);
}

int main() {
    test_basic_operations();
    test_directories();
    test_multiple_files();
    test_data_integrity();
    test_journal_recovery();
    test_no_space();

    printf("Все тесты пройдены!\n");
    return 0;
}




