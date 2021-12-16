#include "Lang.h"

int64_t SimpleHash (const void *data, int len)
{
    int64_t hash        = 0;
    const char *data_ch = (const char *) data;

    for (int byte = 0; byte < len; byte++)
    {
        hash += data_ch[byte] * (byte + 1);
    }

    return hash;
}
