#include "Lang.h"

const int MAX_LEN = 100;

int main (const int argc, const char **argv)
{
    char string[MAX_LEN] = {};
    scanf ("%[^\n]", string);
    int len = strlen (string);

    int64_t hash       = 0;
    int     bytes_read = 1;
    int     words      = 1;

    for (int ch = 0; ch < len; ch++)
    {
        if (isspace (string[ch]))
        {
            words++;
            bytes_read = 1;
            continue;
        }
        hash += string[ch] * bytes_read++;
    }

    printf ("PHRASE (%d, , \"%s\", %ld)\n", words, string, hash);

    return 0;
}
