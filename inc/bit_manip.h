#ifndef BIT_MANIP_H
#define BIT_MANIP_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>


/**
 * @brief Extracts an arbitrary field from a param with mask and pos.
 * @param param The parameter to extract the field from.
 * @param mask The mask of the field.
 * @param pos The pos of the field.
 * @return the field.
*/
size_t extract_field(size_t param, size_t mask, size_t pos);

#endif