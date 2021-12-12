/*****************************************************************//**
 * \file   files.cpp
 * \brief  В файле собраны все функции, связанные с работой с файловой системой
 *********************************************************************/
#include "files.h"

int GetArgs (int argc, const char **argv, Config *curr_config)
{
    while (--argc > 0)
    {
        argv++;

        if (!strncmp (*argv, "-i", 3))
        {
            curr_config->input_file = *(++argv);
            argc--;
        }
        else if (!strncmp (*argv, "-o", 3))
        {
            curr_config->output_file = *(++argv);
            argc--;
        }
        else if (!strncmp (*argv, "-b", 3))
        {
            curr_config->settings |= READ_BASE;
        }
    }
    return 0;
}

long int read_all_lines (File_info *info, const char* file_name)
{
    assert (info);
    assert (file_name);

    char *text_buff = read_file (file_name);

    if (text_buff == NULL)
    {
        return READ_TEXT_FAILED;
    }

    String **strings = (String **) calloc (BUFF_SIZE + 1, sizeof (String *));
    assert (strings);

    String *strings_buff = (String *) calloc (BUFF_SIZE + 1, sizeof (String));
    assert (strings_buff);

    for (int i = 0; i < BUFF_SIZE + 1; i++)
    {
        strings [i] = strings_buff + i;
        assert (strings [i]);
    }

    String **strings_ptr = strings;

    for (char *token = strtok (text_buff, "$\n\r"); token; token = strtok (NULL, "$\n\r"))
    {
      while (isspace(*token)) token++;
      char *token_ptr = token;
      while (*token_ptr != '\n' && *token_ptr) token_ptr++;
      (*strings_ptr)->len = token_ptr - token;
      (*strings_ptr++)->text = token;
    }

    info->text = text_buff;
    info->strs = strings;
    info->strs_buff = strings_buff;
    info->lines_num = strings_ptr - strings;

    return info->lines_num;
}

char *read_file (const char *file_name)
{
    FILE *source = fopen (file_name, "rt");
    if (!source)
    {
        printf ("ERROR: Couldn't open file \"%s\"\n", file_name);
        return NULL;
    }

    char *text = read_to_end (source);

    fclose (source);

    return text;
}

char *read_to_end (FILE *source)
{
    assert (source);

    long unsigned int length = get_len (source);

    char *text_buff = (char *) calloc ( length + 1, sizeof ( char ) );
    assert (text_buff);

    long unsigned int sym_read = fread (text_buff, sizeof (*text_buff), length, source);

    if (sym_read > length)
    {
         free (text_buff);
         printf ("ERROR: Reading text file failed");
         return (NULL);
    }

    // Останавливает дальнейшее чтение, т.к. дальше лежит мусор
    text_buff[sym_read] = '\0';

    return text_buff;
}

long unsigned int get_len (FILE *file)
{
    assert (file);

    fseek (file, 0, SEEK_END);
    long int length = ftell (file);
    fseek (file, 0, SEEK_SET);
    if (length < 0) return 0;
    return (unsigned long int)length;
}

int show_res (File_info *file_text, const char *output_file)
{
    assert (file_text);

    FILE *destination = fopen (output_file, "wt");

    for (int i = 0; i < file_text->lines_num; i++)
    {
        fputs ((file_text->strs [i])->text, destination);

        fputs ("\n", destination);
        if (feof (destination))
        {
            printf ("ERROR: Writing to file failed!");
            free_info (file_text);
            return (WRITING_TEXT_FAILED);
        }
    }
    fclose (destination);
    return 0;
}

void free_info (File_info *info)
{
    assert (info && "Invalid pointer: File_info");

    free (info->text);
    free (info->strs);
    free (info->strs_buff);
}
