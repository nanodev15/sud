#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include <preemptive_set.h>
const char color_table[] = "123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "@&*";


pset_t char2pset(char c) {
    pset_t pset=1;
    int i;
    for(i=0; (i<MAX_COLORS) && (color_table[i] !=c) ; i++)
        ;
    return pset << i;
}

void pset2str(char string[MAX_COLORS + 1], pset_t pset) {
    size_t j;
    pset_t mask = 1;
    size_t k = 0;
    for (j = 0; j < 64; j++) {
        if (pset & mask) {
            string[k] = color_table[j];
            k++;
        }
        mask = mask << 1;
    }
    string[k] = '\0'; // end of string
}

pset_t pset_full(size_t color_range) {
    pset_t mask=1;
    if (color_range >= MAX_COLORS)
        return FULL;
    
       return (pset_t) ((mask<<(color_range)) - 1);
}

pset_t pset_empty(void) {
    return (pset_t) 0;
}

pset_t pset_set(pset_t pset, char c) {
    return pset | char2pset(c);

}

pset_t pset_discard(pset_t pset, char c) {
    return pset & ~char2pset(c);
}

pset_t pset_negate(pset_t pset) {
    return ~pset;
}

pset_t pset_and(pset_t pset1, pset_t pset2) {
    return pset1 & pset2;
}

pset_t pset_or(pset_t pset1, pset_t pset2) {
    return pset1 | pset2;
}

pset_t pset_xor(pset_t pset1, pset_t pset2) {
    return pset1 ^ pset2;
}

bool pset_is_included(pset_t pset1, pset_t pset2) {

    return ((pset1 & pset2) == pset1);
}

bool pset_is_singleton(pset_t pset) {

    return pset && !(pset & (pset - 1));
}

size_t pset_cardinality(pset_t pset) {
    size_t  c=0;
    pset = pset - ((pset >> 1) & (pset_t)(~(pset_t)0/3)); //parrallel count
    pset = (pset & (pset_t)(~(pset_t)0/15*3)) + ((pset >> 2) &//parrallel count
            (pset_t)(~(pset_t)0/15*3));
    pset = (pset + (pset >> 4)) & (pset_t)(~(pset_t)0/255*15); //arrallel count
    c = (pset_t)(pset * ((pset_t)(~(pset_t)0/255))) >> (sizeof(pset_t) - 1) *
            CHAR_BIT; // multiply 0x1010101 and shifting right 24 bits
    return c;
}

pset_t pset_leftmost(pset_t x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
   return (x & ~(x >> 1));
}

pset_t pset_discard2(pset_t pset1, pset_t pset2){
  return pset_and(pset_negate(pset1),pset2);
}

size_t trailingZeroCount(pset_t pset) {
    static const int lookup67[67 + 1] = {
        64, 0, 1, 39, 2, 15, 40, 23,
        3, 12, 16, 59, 41, 19, 24, 54,
        4, -1, 13, 10, 17, 62, 60, 28,
        42, 30, 20, 51, 25, 44, 55, 47,
        5, 32, -1, 38, 14, 22, 11, 58,
        18, 53, 63, 9, 61, 27, 29, 50,
        43, 46, 31, 37, 21, 57, 52, 8,
        26, 49, 45, 36, 56, 7, 48, 35,
        6, 34, 33, -1
    };
    return lookup67[(pset & -pset) % 67];
}
