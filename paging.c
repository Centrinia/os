/* paging.c */

#include "util.h"

struct page_table_entry {
    uint32_t present:1;
    /* 0 if writes are not allowed; section 4.6 */
    uint32_t write:1;
    uint32_t user:1;
    uint32_t write_through:1;
    uint32_t cache_disable:1;
    /* section 4.8 */
    uint32_t accessed:1;
    /* section 4.8 */
    uint32_t dirty:1;
    /* The page size must be 1 for 4MB pages. */
    uint32_t page_size:1;

    /* section 4.10 */
    uint32_t global:1;
     uint32_t:3;

    uint32_t address:20;
};
struct page_directory_table {
    uint32_t present:1;
    /* 0 if writes are not allowed; section 4.6 */
    uint32_t write:1;
    uint32_t user:1;
    uint32_t write_through:1;
    uint32_t cache_disable:1;
    /* section 4.8 */
    uint32_t accessed:1;
     uint32_t:1;
    uint32_t zero:1;
     uint32_t:4;
    uint32_t address:20;
};

struct page_directory_page {
    uint32_t present:1;
    /* 0 if writes are not allowed; section 4.6 */
    uint32_t write:1;
    uint32_t user:1;
    uint32_t write_through:1;
    uint32_t cache_disable:1;
    /* section 4.8 */
    uint32_t accessed:1;
    /* section 4.8 */
    uint32_t dirty:1;
    /* The page size must be 1 for 4MB pages. */
    uint32_t page_size:1;

    /* section 4.10 */
    uint32_t global:1;
     uint32_t:3;

    /* section 4.9.2 */
    uint32_t pat:1;

    uint32_t address_high:4;

    uint32_t zero:5;

    uint32_t address:10;
};

#define PAGE_DIRECTORY_ADDRESS	((640*1024-4096) & (-4096))
#define PAGE_TABLE_0_ADDRESS	((640*1024-4096*3) & (-4096))

static void enable_paging(void *page_directory)
{
    asm volatile ("mov %0,%%cr3"::"a" ((uint32_t) page_directory & -4096));
    /* Enable paging. */
    uint32_t cr = 0;
    asm volatile ("mov %%cr0, %0":"=r" (cr));
    cr |= 1UL << 31;
    asm volatile ("mov %0,%%cr0"::"r" (cr));


    /* Enable 4MB pages. */
    asm volatile ("mov %%cr4, %0":"=r" (cr));
    cr |= 1UL << 4;
    asm volatile ("mov %0,%%cr4"::"r" (cr));
}

void setup_paging()
{
    struct page_directory_page *page_directory =
	(struct page_directory_page *) PAGE_DIRECTORY_ADDRESS;

    struct page_directory_page big_page = {
	.present = 1,
	.write = 1,
	.user = 0,
	.write_through = 0,
	.cache_disable = 0,
	.accessed = 0,
	.dirty = 0,
	/* These are 4MB pages. */
	.page_size = 1,
    .global = 0,.pat = 0,.zero = 0,.address_high = 0,};
    struct page_table_entry small_page = {
	.present = 1,
	.write = 1,
	.user = 0,
	.write_through = 0,
	.cache_disable = 0,
	.accessed = 0,
	.page_size = 0,
    .global = 0,};

    for (int i = 0; i < 1024; i++) {
	page_directory[i] = big_page;
	page_directory[i].address = i;
    }
    struct page_directory_table *page_tables =
	(struct page_directory_table *) PAGE_DIRECTORY_ADDRESS;

    struct page_directory_table page_directory_entry = {
	.present = 1,
	.write = 1,
	.user = 0,
	.write_through = 0,
	.cache_disable = 0,
	.accessed = 0,
	.zero = 0,
    };


    page_tables[0] = page_directory_entry;
    page_tables[0].address = PAGE_TABLE_0_ADDRESS >> 12;
    page_tables[1] = page_directory_entry;
    page_tables[1].address = (PAGE_TABLE_0_ADDRESS >> 12) + 1;


    struct page_table_entry *page_table =
	(struct page_table_entry *) PAGE_TABLE_0_ADDRESS;
    for (int i = 0; i < 2048; i++) {
	page_table[i] = small_page;
	page_table[i].address = i;
    }
    enable_paging(page_directory);


    // TODO: Test page fault.
    //page_tables[0].present = 1;
    /*for(int i=0;i<1024;i++) {
       page_table[1024+i].present = 0;
       } */
    //page_table[1024].present = 0;

    page_tables[1].present = 0;
    //page_tables[1].write = 1;
}
