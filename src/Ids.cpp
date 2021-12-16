#include "Lang.h"

#define IDS (*IdsArr)
#define NUM (*IdsNum)

static int idCap = INIT_IDS_NUM;

int AddId (Id **IdsArr, int *IdsNum, int64_t hash, char isConst)
{
    Id new_id      = {};
    new_id.hash    = hash;
    new_id.isConst = isConst;

    $ LogMsg ("\tnew hash = %ld\n", hash);

    IDS[NUM++] = new_id;

    if (NUM >= idCap)
    {
        $ Id *tmp = (Id *) realloc (IDS, (size_t) idCap * 2 * sizeof (Id));
        assert (tmp);

        IDS = tmp;
        idCap *= 2;
    }

    return NUM;
}

int FindId (Id **IdsArr, int *IdsNum, int64_t hash)
{
    for (int id = 0; id < NUM; id++)
    {
        if (IDS[id].hash == hash)
        {
            return id;
        }
    }

    return -1;
}

int RmId (Id **IdsArr, int *IdsNum, int num)
{
    if (NUM < 1) return 0;

    for (int id = 0; id < num; id++)
    {
        IDS[NUM--] = {};
    }

    if (NUM < idCap / 4)
    {
        $ Id *tmp = (Id *) realloc (IDS, (size_t) idCap / 2 * sizeof (Id));
        assert (tmp);

        IDS = tmp;
        idCap /= 2;
    }

    return NUM;
}
