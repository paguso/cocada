
#include "alphabet.h"
#include "dna.h"

alphabet *dna_ab_new() {
    return alphabet_new(4, "acgt");
}
