#ifndef PGTBL_H
#define PGTBL_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define PAGESIZE 4096UL
#define LEVELS 2U
#define VA_OFFSET_MASK 0xFFFUL
#define VA_VPN_MASK 0x3FFUL
#define PGTBL_SIZE PAGESIZE
#define PGTBL_NUM_ENTRIES 1024UL
#define PPN_MASK 0x3FFFFFUL
#define PPN_OFFSET 12U
#define PTESIZE 4U

//PTE bit fields
#define PTE_V_POS 0
#define PTE_V_MASK 0x1
#define PTE_R_POS 1
#define PTE_R_MASK 0x1
#define PTE_W_POS 2
#define PTE_W_MASK 0x1
#define PTE_X_POS 3
#define PTE_X_MASK 0x1
#define PTE_U_POS 4
#define PTE_U_MASK 0x1
#define PTE_G_POS 5
#define PTE_G_MASK 0x1
#define PTE_A_POS 6
#define PTE_A_MASK 0x1
#define PTE_D_POS 7
#define PTE_D_MASK 0x1
#define PTE_RSW_POS 8
#define PTE_RSW_MASK 0x2

typedef size_t pte_t;

typedef struct {
    pte_t entries[PGTBL_NUM_ENTRIES] __attribute__((aligned(PAGESIZE)));
} page_table_t;

typedef struct {
    page_table_t* root_pgtbl;
    page_table_t* secondary_pgtbl;
} sv32_pgtbl_set_t;

typedef struct {
    bool valid;
    bool read;
    bool write;
    bool execute;
    bool user;
    bool global;
    bool accessed;
    bool dirty;
} pte_perms_t;

/**
 * @brief Performs a page table walk to translate VA to PA.
 * @param VA the virtual address.
 * @return Physical address.
*/
size_t pgtbl_walk(size_t VA, page_table_t* root_pgtbl);

/**
 * @brief Extracts va.offset from a va.
 * @param VA the virtual address.
 * @return va.offset.
*/
size_t offset_from_va(size_t VA);

/**
 * @brief Maps a VA->PA on a 4k page into root_pgtbl. VA gets offset bits removed to map entire 4k physical page.
 * @param VA The virtual address to be mapped to the page.
 * @param PA The physical address of the page.
 * @param perms The PTE permissions for the leaf PTE.
 * @param pgtbls Pointer to the page table set to map into.
 * @return A pointer to the page table.
*/
bool map_4k_page(size_t VA, size_t PA, pte_perms_t perms, sv32_pgtbl_set_t* pgtbls);

/**
 * @brief Allocates a new page table.
 * @return A pointer to the page table.
*/
page_table_t *allocate_page_table();

/**
 * @brief Creates a pointer to the next level page table.
 * @param ppn_to_next_level The PPN of the next level page table.
 * @return A pointer pte.
*/
pte_t make_pte_ptr(size_t ppn_to_next_level);

/**
 * @brief Creates a leaf PTE.
 * @param perms The permissions of the PTE.
 * @return A leaf pte.
*/
pte_t make_leaf_pte(size_t ppn, pte_perms_t perms);

/**
 * @brief Extracts the ppn from a page table or pte.
 * @param root_pgtbl The PTE or page table address.
 * @return the PPN.
*/
size_t get_ppn(intptr_t ppn_object);

/**
 * @brief Extracts the VPN from a VA corresponding to level.
 * @param VA the virtual address.
 * @param level the level in va.vpn[level].
 * @return the VPN.
*/
size_t vpn_from_va(size_t VA, int level);

#endif