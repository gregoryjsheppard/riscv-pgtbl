#include "bit_manip.h"

size_t extract_field(size_t param, size_t mask, size_t pos){
    return (param & (mask << pos)) >> pos;
}