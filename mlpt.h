#ifndef MLPT_H_
#define MLPT_H_
/**
 * Page table base register.
 * Declared here so tester code can look at it; because it is extern
 * you'll need to define it (without extern) in exactly one .c file.
 */
extern size_t ptbr;

/**
 * Given a virtual address, return the physical address.
 * Return a value consisting of all 1 bits
 * if this virtual address does not have a physical address.
 */
size_t translate(size_t va);

size_t generate_page(size_t address_to_insert, size_t align, size_t size);

/**
 * Use posix_memalign to create page tables sufficient to have a mapping
 * between the given virtual address and some physical address. If there
 * already is such a page, does nothing.
 */
void page_allocate(size_t va);

typedef struct stack stack;

void push (struct stack **top, size_t value);

size_t pop(struct stack **top);

void page_deallocate(size_t va);

#endif
