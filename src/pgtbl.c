#include "pgtbl.h"
#include "bit_manip.h"

size_t vpn_from_va(size_t VA, int level) {
    assert(level < LEVELS);
    //VPN[1] VPN[0] OFFSET
    //  10     10     12
    size_t VA_VPN_SHIFT = level*10 + 12;
    return (VA & (VA_VPN_MASK << VA_VPN_SHIFT)) >> VA_VPN_SHIFT;
}

size_t extract_ppn_level_from_pte(pte_t pte, int level) {
    assert(level < LEVELS);
    //PPN[1] PPN[0] PERMS BITS
    //  12     10     10
    if(level) {
        return extract_field((size_t)pte, PTE_PPN_1_MASK, PTE_PPN_1_POS);
    } else {
        return extract_field((size_t)pte, PTE_PPN_0_MASK, PTE_PPN_0_POS);
    }
}

size_t extract_ppn_level_from_pa(size_t pa, int level) {
    assert(level < LEVELS);
    //PPN[1] PPN[0] PERMS BITS
    //  12     10     10
    if(level) {
        return extract_field(pa, PA_PPN_1_MASK, PA_PPN_1_POS);
    } else {
        return extract_field(pa, PA_PPN_0_MASK, PA_PPN_0_POS);
    }
}

size_t offset_from_va(size_t VA) {
    return VA & VA_OFFSET_MASK;
}

size_t get_ppn(intptr_t ppn_object) {
    return (ppn_object & (PPN_MASK << PPN_OFFSET)) >> PPN_OFFSET;
}

void page_fault() {
    exit(1);
}

void print_permissions(pte_t pte) {
    printf("Permissions: ");
    if(extract_field(pte, PTE_R_MASK, PTE_R_POS)) {
        printf("r");
    }
    if(extract_field(pte, PTE_X_MASK, PTE_X_POS)) {
        printf("x");
    }
    if(extract_field(pte, PTE_W_MASK, PTE_W_POS)) {
        printf("w");
    }
    printf("\n");
}

page_table_t* allocate_page_table() {
    page_table_t *pgtbl;
    if(posix_memalign((void*)&pgtbl, PAGESIZE, PAGESIZE)) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    
    for (int i = 0; i < 1024; ++i) {
        pgtbl->entries[i] = (pte_t)0;
    }
    return pgtbl;
}

pte_t make_pte_ptr(size_t ppn_to_next_level) {
    return ppn_to_next_level*PAGESIZE+(PTE_V_MASK << PTE_V_POS);
}

pte_t make_leaf_pte(size_t ppn, pte_perms_t perms, size_t level) {
    //represent perms struct as a bit vector
    size_t pte_perms = (perms.valid << PTE_V_POS)+(perms.read << PTE_R_POS)+(perms.write << PTE_W_POS)
                        +(perms.execute << PTE_X_POS)+(perms.user << PTE_U_POS) +(perms.global << PTE_G_POS)
                        +(perms.accessed << PTE_A_POS)+(perms.dirty << PTE_D_POS);
    if(level) {
        return (ppn<<PTE_PPN_1_POS)+pte_perms;
    } else {
        return ppn*PAGESIZE+pte_perms;
    }
}

bool map_4m_page(size_t VA, size_t PA, pte_perms_t perms, sv32_pgtbl_set_t* pgtbls) {
    printf("Creating a 4M page mapping VA(0x%x)->PA(0x%x).\n", VA, PA);
    size_t vpn = vpn_from_va(VA, LEVELS-1);
    pte_t pte_0 = pgtbls->root_pgtbl->entries[vpn];
    printf("Root_pgtbl[0x%x]=0x%x\n", vpn, pte_0);
    if(!(bool)extract_field(pte_0, PTE_V_MASK, PTE_V_POS)) {
        pte_t leaf_pte = make_leaf_pte(extract_ppn_level_from_pa(PA, LEVELS-1), perms, LEVELS-1);
        pgtbls->root_pgtbl->entries[vpn] = leaf_pte;
        printf("=>Root_pgtbl[0x%x]=0x%x\n", vpn, leaf_pte);
        return true;
    }
    printf("A PTE is currently mapped into this space! Aborting.\n");
    return false;
}

bool map_4k_page(size_t VA, size_t PA, pte_perms_t perms, sv32_pgtbl_set_t* pgtbls) {
    printf("Creating a 4k page mapping VA(0x%x)->PA(0x%x).\n", VA, PA);
    size_t vpn_0 = vpn_from_va(VA, LEVELS-1);
    pte_t pte_0 = pgtbls->root_pgtbl->entries[vpn_0];
    printf("Root_pgtbl[0x%x]=0x%x\n", vpn_0, pte_0);
    if(!(bool)extract_field(pte_0, PTE_V_MASK, PTE_V_POS)) {
        pte_t ptr_pte = make_pte_ptr(get_ppn((intptr_t)pgtbls->secondary_pgtbl));
        pgtbls->root_pgtbl->entries[vpn_0] = ptr_pte;
        printf("=>Root_pgtbl[0x%x]=0x%x\n", vpn_0, ptr_pte);
    }
    size_t vpn_1 = vpn_from_va(VA, LEVELS-2);
    pte_t pte_1 = pgtbls->secondary_pgtbl->entries[vpn_1];
    printf("secondary_pgtbl[0x%x]=0x%x\n", vpn_1, pte_1);
    if(!(bool)extract_field(pte_1, PTE_V_MASK, PTE_V_POS)) {
        pte_t leaf_pte = make_leaf_pte(get_ppn(PA), perms, 0);
        pgtbls->secondary_pgtbl->entries[vpn_1] = leaf_pte;
        printf("=>secondary_pgtbl[0x%x]=0x%x\n", vpn_1, leaf_pte);
        return true;
    }
    printf("VA->PA mapping already exists!\n");
    return false;
}

size_t pgtbl_walk(size_t VA, page_table_t* root_pgtbl) {
    printf("Starting page table walk, VA=0x%x\n", VA);
    pte_t leaf_pte = 0;
    page_table_t* current_pgtbl = root_pgtbl;
    size_t i = LEVELS - 1;
    for(; i >= 0; i--) {
        if(i < 0) {
            printf("Page fault: i < 0\n");
            page_fault();
        }
        pte_t pte =  current_pgtbl->entries[vpn_from_va(VA, i)];
        printf("i=%d, pte=0x%x\n", i, pte);
        bool v_bit = (bool)extract_field(pte, PTE_V_MASK, PTE_V_POS);
        bool r_bit = (bool)extract_field(pte, PTE_R_MASK, PTE_R_POS);
        bool w_bit = (bool)extract_field(pte, PTE_W_MASK, PTE_W_POS);
        bool x_bit = (bool)extract_field(pte, PTE_X_MASK, PTE_X_POS);
        //printf("PTE: 0x%x\n", pte);
        //test if the pte is valid, if not, raise page fault correspondin g to original access type.
        if(!v_bit || (!r_bit && w_bit)) {
            printf("PAGE FAULT: Faulting PTE=0x%x @ %p\n", pte, &pte);
            page_fault();
        }
        //check if this is a leaf pte
        if(r_bit || x_bit) {
            leaf_pte = pte;
            break;
        }
        //change a to pte.ppn, then continue until the leaf is found
        current_pgtbl = (page_table_t*)(get_ppn(pte)*PAGESIZE);
    }
    //check if this is a misaligned super page and raise a page fault
    if(i > 0 && extract_ppn_level_from_pte(leaf_pte, i-1) != 0) {
        printf("PAGE FAULT: Misaligned superpage. PTE=0x%x @ %p\n", leaf_pte, &leaf_pte);
        page_fault();
    }

    //at this point, the leaf PTE has been found and is stored in leaf_pte.
    //build PA from the VA and PTE
    size_t pa = get_ppn(leaf_pte)*PAGESIZE + offset_from_va(VA);
    printf("Translation Found! VA(0x%x)->PA(0x%x), ", VA, pa);
    print_permissions(leaf_pte);
    return pa;
}
