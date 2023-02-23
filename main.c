#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mlpt.h"

int main() {
    // 0 pages have been allocated
    assert(ptbr == 0);
    printf(">>>>>ASSERTED ptbr == 0\n");

    page_allocate(0x456789abcdef);
    page_allocate(0x456789abcdef);
    // 5 pages have been allocated: 4 page tables and 1 data
    printf(">>>>>Allocation 1/4 successful\n");
    assert(ptbr != 0);
    printf(">>>>>ASSERTED ptbr != 0\n");


    page_allocate(0x456789abcd00);
    // no new pages allocated (still 5)
    printf(">>>>>Allocation 2/4 successful\n");


    int *p1 = (int *)translate(0x456789abcd00);
    printf(">>>>>Translate 1/4 successful\n");
    *p1 = 0xaabbccdd;
    short *p2 = (short *)translate(0x456789abcd02);
    printf(">>>>>Translate 2/4 successful\n");
    printf(">>>>>%04hx\n", *p2); // prints "aabb\n"
    printf(">>>>>aabb if successful\n");

    assert(translate(0x456789ab0000) == 0xFFFFFFFFFFFFFFFF);
    printf(">>>>>Translate 3/4 successful\n");

    
    page_allocate(0x456789ab0000);
    // 1 new page allocated (now 6; 4 page table, 2 data)
    printf(">>>>>Allocation 3/4 successful\n");


    assert(translate(0x456789ab0000) != 0xFFFFFFFFFFFFFFFF);
    printf(">>>>>Translate 4/4 successful\n");


    page_allocate(0x456780000000);
    // 2 new pages allocated (now 8; 5 page table, 3 data)
    printf(">>>>>Allocation 4/4 successful\n");

    printf(">>>>>>>>>>Test Successful\n");
}
