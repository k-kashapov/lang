#define PHRASE(words, name, text, ph_hash) name,
enum Phrases
{
    #include "Phrases.h"
};
#undef PHRASE
