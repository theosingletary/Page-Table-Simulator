#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;
//Returns a number where the bottom POBITS bits are all set to 1
size_t pobits_mask = (1 << (POBITS & 0x1F)) + ~(POBITS >> 5);
//vpn_size - #bits for each VPN 
size_t vpn_size = POBITS-3;
//Returns a number where the bottom vpn_size bits are all set to 1 
size_t levels_mask = (1 << ((POBITS-3) & 0x1F)) + ~((POBITS-3) >> 5);

size_t translate(size_t va) {
    size_t ret = 0xFFFFFFFFFFFFFFFF;
    if (ptbr == 0) { //base case if no page table initialized
        return ret;
    }
    
    size_t page_offset = va & pobits_mask;
    size_t vpn_master = va >> POBITS;
    size_t local_levels_mask = levels_mask << ((LEVELS-1) * vpn_size);
    size_t ptbr_local = ptbr;
    size_t current_vpn;
    size_t current_address;

    for (int i = 1; i <= LEVELS; ++i) {
        current_vpn = (local_levels_mask & vpn_master) >> ((LEVELS-i) * vpn_size);
        current_address = ptbr_local + (8 * current_vpn);

        if ((*((size_t*) current_address) & 1) == 1) {
            ptbr_local = (((*((size_t*) current_address)) >> 1) << 1);//shift out the valid bit in PTE
        }
        else {
            return ret;//if invalid page, return -1
        }

        local_levels_mask = local_levels_mask >> vpn_size;//shift levels mask to next level
    }
    ret = ptbr_local + page_offset; //add page offset
    return ret;
}

size_t generate_page(size_t address_to_insert, size_t align, size_t size) {
    void *pt;
    posix_memalign(&pt, align, size);
    size_t ret_ptbr = (size_t) pt;

    for (int i = 0; i < size/8; ++i) {
        *(((size_t*) ret_ptbr) + i) = 0;
    }

    if (ptbr == 0) {
        return ret_ptbr;
    }
    else {
        size_t validated = ret_ptbr | 1;
        *((size_t*) address_to_insert) = validated;
        return ret_ptbr;
    }
}

void page_allocate(size_t va) {
    size_t page_size = pow(2, POBITS);

    if (ptbr == 0) {//create initial page table
        ptbr = generate_page(0, page_size, page_size);
    }

    size_t vpn_master = va >> POBITS;
    size_t local_levels_mask = levels_mask << ((LEVELS-1) * vpn_size);
    size_t ptbr_local = ptbr;
    size_t current_vpn;
    size_t current_address;

    for (int i = 1; i <= LEVELS; ++i) {
        current_vpn = (local_levels_mask & vpn_master) >> ((LEVELS-i) * vpn_size);
        current_address = ptbr_local + (8 * current_vpn);

        if ((*((size_t*) current_address) & 1) == 1) {//if page already exists
            ptbr_local = (((*((size_t*) current_address)) >> 1) << 1);//shift out the valid bit in PTE
        }

        else {//create new page
            ptbr_local = generate_page(current_address, page_size, page_size);
        }
        local_levels_mask = local_levels_mask >> vpn_size;//shift levels mask to next level
    }
}
