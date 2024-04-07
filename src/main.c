#include "pgtbl.h"

const pte_perms_t READ_ONLY_PAGE = {1, 1, 0, 0, 0, 0, 0, 0};
const pte_perms_t READ_WRITE_PAGE = {1, 1, 1, 0, 0, 0, 0, 0};
const pte_perms_t EXECUTE_ONLY_PAGE = {1, 0, 0, 1, 0, 0, 0, 0};
const pte_perms_t READ_EXECUTE_PAGE = {1, 1, 0, 1, 0, 0, 0, 0};
const pte_perms_t READ_WRITE_EXECUTE_PAGE = {1, 1, 1, 1, 0, 0, 0, 0};

/**
 * @brief Runs the main program.
*/
int main(int argc, char** argv) {
    sv32_pgtbl_set_t pgtbls;
    pgtbls.root_pgtbl = allocate_page_table();
    pgtbls.secondary_pgtbl = allocate_page_table();
    printf("root pgtbl ppn: %p\n", pgtbls.root_pgtbl);
    printf("secondary pgtbl ppn: %p\n", pgtbls.secondary_pgtbl);
    map_4k_page(0x12345678, 0x87654321, READ_WRITE_EXECUTE_PAGE, &pgtbls);
    pgtbl_walk(0x12345678, pgtbls.root_pgtbl);
    return 0;
}