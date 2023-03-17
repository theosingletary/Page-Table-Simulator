#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mlpt.h"
#include "config.h"

size_t ptbr = 0;
//Returns a number where the bottom POBITS bits are all set to 1 - taken from answer key to CSO1 HW1 "bottom" answer key
size_t pobits_mask = (1 << (POBITS & 0x1F)) + ~(POBITS >> 5);
//vpn_size - #bits for each VPN - https://piazza.com/class/lcrttbhw5ll3ef/post/107 
size_t vpn_size = POBITS-3;
//Returns a number where the bottom vpn_size bits are all set to 1 - taken from answer key to CSO1 HW1 "bottom" answer key
size_t levels_mask = (1 << ((POBITS-3) & 0x1F)) + ~((POBITS-3) >> 5);

//consulted enh4bn, Elliot Hansen
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
            ptbr_local = (((*((size_t*) current_address)) >> 1) << 1);
        }
        else {
            return ret;
        }

        local_levels_mask = local_levels_mask >> vpn_size;
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

//consulted enh4bn, Elliot Hansen
void page_allocate(size_t va) {
    size_t page_size = pow(2, POBITS);

    if (ptbr == 0) {
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
        
        if ((*((size_t*) current_address) & 1) == 1) {
            ptbr_local = (((*((size_t*) current_address)) >> 1) << 1);
        }
        else {
            ptbr_local = generate_page(current_address, page_size, page_size);
        }
        local_levels_mask = local_levels_mask >> vpn_size;
    }
}

struct stack {
	size_t data;
	struct stack *bottom;
};

void push (struct stack **top, size_t value){
	struct stack *node = malloc(sizeof(stack));
	node->data = value;
	node->bottom = *top;
	*top = node;
}

size_t pop(struct stack **top) {
	struct stack *temp = *top;
	size_t value = temp->data;
	*top = temp->bottom;
	free(temp);
	return value;
}

void page_deallocate(size_t va) {
    size_t page_size = pow(2, POBITS);

    if (ptbr == 0) {
        return;
    }

    size_t vpn_master = va >> POBITS;
    size_t local_levels_mask = levels_mask << ((LEVELS-1) * vpn_size);
    size_t ptbr_local = ptbr;
    size_t current_vpn;
    size_t current_address;
    stack *head = NULL;

    for (int i = 1; i <= LEVELS; ++i) {
        current_vpn = (local_levels_mask & vpn_master) >> ((LEVELS-i) * vpn_size);
        current_address = ptbr_local + (8 * current_vpn);

        if (i == LEVELS) {//when at index of final page table
            for (int j = LEVELS; j >= 1; --j){
                if ((*((size_t*) current_address) & 1) == 1) {
                    size_t address_to_free = ((current_address >> 1) << 1);
                    free((void*) address_to_free);
                    *((size_t*) current_address) = 0;
                    int valid_counter = 0;
                    for (int k = 0; k < page_size/8; ++k) {
                        if ((*(((size_t*) ptbr_local) + k) & 1) == 1){
                            ++valid_counter;
                        }
                    }
                    if (valid_counter > 0) {
                        return;
                    }
                    if ((ptbr_local == ptbr) && (valid_counter == 0)) {
                        free((void*) ptbr);
                        ptbr = 0;
                        return;
                    }
                    current_address = pop(&head);
                    ptbr_local = pop(&head);
                }
                else {
                    return;
                }
            }
        }
        
        push(&head, ptbr_local);
        push(&head, current_address);

        if ((*((size_t*) current_address) & 1) == 1) {
            ptbr_local = (((*((size_t*) current_address)) >> 1) << 1);
        }
        else {
            return;
        }
        local_levels_mask = local_levels_mask >> vpn_size;
    }
}