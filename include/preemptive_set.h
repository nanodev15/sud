#ifndef PREEMPTIVE_SET_H
#define PREEMPTIVE_SET_H
#define FULL 0xFFFFFFFFFFFFFFFF   // Full pset
#define MAX_COLORS 64              // max colors


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// color table
extern const char color_table[];
// definition of pset
typedef uint64_t pset_t;
// turn pset to String
void pset2str(char string[MAX_COLORS +1], pset_t pset);
// turn char to pset
pset_t char2pset(char c);
// psets intersection
pset_t pset_and(pset_t pset1, pset_t pset2);
// number of colors in pset
size_t pset_cardinality (pset_t pset);
//substract color that match char c from pset
pset_t pset_discard (pset_t pset,char c);
//substract pset1 from pset2
pset_t pset_discard2 (pset_t pset1,pset_t pset2);
// empty pset
pset_t pset_empty(void);
// full pset
pset_t pset_full (size_t color_range);
// test if pset1 is included in pset 2
bool pset_is_included (pset_t pset1, pset_t pset2);
// test if pset has only one color
bool pset_is_singleton(pset_t pset);
// switch pset colors
pset_t pset_negate(pset_t pset);
//  inclusive union of pset1 and pset 2
pset_t pset_or(pset_t pset1, pset_t pset2);
 // set color matching char c in pset
pset_t pset_set (pset_t pset, char c);
// exclusive union of pset 1 and pset 2
pset_t pset_xor (pset_t pset1, pset_t pset2);
// return the leftmost bit set of pset
pset_t pset_leftmost(pset_t pset);
// return the number of trailing zeros in pset
size_t trailingZeroCount(pset_t pset);
#endif