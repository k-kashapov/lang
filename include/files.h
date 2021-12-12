#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

/// <summary>
/// Максимальное количество строк, которые можно прочиать из файла
/// </summary>
const int BUFF_SIZE = 500;

enum FilesExitCodes
{
  READ_TEXT_FAILED =    -1,
  WRITING_TEXT_FAILED = -2,
  OPEN_FILE_FAILED =    -3
};

enum LANG_OPTNS
{
    READ_BASE = 0x01,
};

/**
 * \brief Структура, содержащая строку и её длину
 */
struct String
{
    char *text;
    long int len;
};

/**
 * \brief Структура, содержащая конфигурацию программы
 *
 * \param input_file  Имя файла, откуда производится чтение
 * \param output_file Имя файла для записи
 */
struct Config
{
    const char *input_file  = "base.txt";
    const char *output_file = "base_new.txt";
    int64_t     settings    = 0;
};

/**
 * \brief Структура, содержащая в себе полный текст файла и информацию о нём
 *
 * \param text      Полный текст файла
 * \param str_ptrs  Массив указателей на строки в файле
 * \param lines_num Количество строк
 */
struct File_info
{
    char *text;
    String **strs;
    long int  lines_num;
    String *strs_buff;
};

/**
 * \brief Читает все строки из файла
 *
 * \param  info         Указатель в который будет записана информация о файле
 * \param  file_name    Имя файла, который будет прочитан
 * \return              Количество прочитанных строк
 */
long int read_all_lines (File_info *info, const char *file_name);

/**
 * \brief Читает файл до конца, возвращает указатель на буфер текста
 */
char *read_file (const char *file_name);

/**
 * \brief Читает файл, помещает все символы в буфер
 *
 * \param  source Файл, который нужно прочитать
 * \return        Указатель на буфер
 */
char* read_to_end (FILE *source);

/**
 * \brief Возвращает количество символов в файле
 *
 * \param  file Указатель на файл
 * \return      Длина файла
 */
long unsigned int get_len (FILE *file);

/**
 * \brief Записывает все строки из структуры source в файл output_file
 *
 * \param source      Структура, откуда будут напечатаны строки
 * \param output_file Название файла, в который необходимо напечатать строки
 */
int show_res (File_info *source, const char * output_file);

/**
 * \brief Очищает строки, содержащиеся в структуре info
 *
 * \param info Структура, память которой будет очищена
 */
void free_info (File_info *info);

int GetArgs (int argc, const char **argv, Config *curr_config);

#endif
