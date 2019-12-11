
#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

static BYTE _ram[RAM_SIZE];
//RAM_SIZE = 1024^2 B
static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct page_table_t * get_page_table(
		addr_t index, 	// Segment level index
		struct seg_table_t * seg_table) { // first level table
	
	/*
	 * TODO: Given the Segment index [index], you must go through each
	 * row of the segment table [seg_table] and check if the v_index
	 * field of the row is equal to the index
	 *
	 * */

	int i;
	for (i = 0; i < seg_table->size; i++) {
		// Enter your code here
		if(seg_table->table[i].v_index == index){
			return seg_table->table[i].pages;
		}
	}
	return NULL;

}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */
static int translate(
		addr_t virtual_addr, 	// Given virtual address
		addr_t * physical_addr, // Physical address to be returned
		struct pcb_t * proc) {  // Process uses given virtual address

	/* Offset of the virtual address */
	addr_t offset = get_offset(virtual_addr);
	/* The first layer index */
	addr_t first_lv = get_first_lv(virtual_addr);
	/* The second layer index */
	addr_t second_lv = get_second_lv(virtual_addr);
	
	/* Search in the first level */
	struct page_table_t * page_table = NULL;
	page_table = get_page_table(first_lv, proc->seg_table);
	if (page_table == NULL) {
		return 0;
	}

	int i;
	for (i = 0; i < page_table->size; i++) {
		if (page_table->table[i].v_index == second_lv) {
			/* TODO: Concatenate the offset of the virtual addess
			 * to [p_index] field of page_table->table[i] to 
			 * produce the correct physical address and save it to
			 * [*physical_addr]  */
			 *physical_addr = (page_table->table[i].p_index << 10) | offset;
			return 1;
		}
	}
	return 0;	
}

addr_t alloc_mem(uint32_t size, struct pcb_t * proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;
	/* TODO: Allocate [size] byte in the memory for the
	 * process [proc] and save the address of the first
	 * byte in the allocated memory region to [ret_mem].
	 * */

	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE + 1 :
		size / PAGE_SIZE; // Number of pages we will use
	int mem_avail = 0; // We could allocate new memory region or not?
	
	printf("ALLOC size: %d \n", num_pages);

	/* First we must check if the amount of free memory in
	 * virtual address space and physical address space is
	 * large enough to represent the amount of required 
	 * memory. If so, set 1 to [mem_avail].
	 * Hint: check [proc] bit in each page of _mem_stat
	 * to know whether this page has been used by a process.
	 * For virtual memory space, check bp (break pointer).
	 * */

	int free_frame[1024]; // store the index of free_using space in mem
	// Check Virtual Address
	if( RAM_SIZE - (proc->bp) >= num_pages * PAGE_SIZE){
		//printf(" H ");
		// Check Physical Address
		int num_zero_mem = 0;
		for ( int i = 0; i < (1 << 10); i++){
			if ( _mem_stat[i].proc == 0 ) {
				free_frame[num_zero_mem] = i;
				num_zero_mem++;
				if (num_zero_mem >= num_pages) {
					mem_avail = 1;
					//printf(" R1 & num_zeo_mem: %d \n", num_pages);
					break;
				}
			}
		}
	}

	if (mem_avail) {
		//printf(" R2 \n");
		/* We could allocate new memory region to the process */
		ret_mem = proc->bp;
		proc->bp += num_pages * PAGE_SIZE;
		/* Update status of physical pages which will be allocated
		 * to [proc] in _mem_stat. Tasks to do:
		 * 	- Update [proc], [index], and [next] field
		 * 	- Add entries to segment table page tables of [proc]
		 * 	  to ensure accesses to allocated memory slot is
		 * 	  valid. */

        int fla = -1;
		int firs_lv;
		int sec_lv;
		int temp = ret_mem;
		for(int i = 0; i < num_pages; i++){

			// [proc]
			_mem_stat[ free_frame[i] ].proc = proc->pid;
			// [index]                      
			_mem_stat[ free_frame[i] ].index = i;
			// [next]
			int temp_index = free_frame[i];
			if(i < num_pages - 1)
				_mem_stat[temp_index].next = free_frame[i + 1];
			else _mem_stat[temp_index].next = -1;
			// [proc->seg_table]

			//if ( get_second_lv(ret_mem) == 0 )
			//printf("reg_size: %d \n", firs_lv);
			if(proc->seg_table->size != 0) firs_lv = proc->seg_table->size-1;
			else firs_lv = 0;
			
			if( get_second_lv(ret_mem) == 0) {
				proc->seg_table->table[firs_lv + 1].pages = 
					(struct seg_table_t*)malloc(sizeof(struct seg_table_t));
				fla = firs_lv;
				proc->seg_table->size++;
				printf("C0 \n");
			}
			else if( get_second_lv(ret_mem) == 1 && get_first_lv(ret_mem) == 0) {
				proc->seg_table->table[firs_lv].pages = 
					(struct seg_table_t*)malloc(sizeof(struct seg_table_t));
					// proc->seg_table->table[0].pages->table[0].v_index = 0;
					// proc->seg_table->table[0].pages->table[0].p_index = -1;
					proc->seg_table->table[firs_lv].pages->size = 1;
					proc->seg_table->size=1;
					printf("C1 \n");
			}
			
			// if( (ret_mem > 1024) && (get_first_lv(ret_mem) != get_first_lv(ret_mem + PAGE_SIZE) ) ){
			// 	proc->seg_table->size++;
			// }
			
			//printf("reg_size: %d \n", firs_lv);
			ret_mem += PAGE_SIZE;
			sec_lv = proc->seg_table->table[firs_lv].pages->size;
			proc->seg_table->table[firs_lv].v_index = firs_lv;
			proc->seg_table->table[firs_lv].pages->table[sec_lv].v_index = 
				proc->seg_table->table[firs_lv].pages->size;
			proc->seg_table->table[firs_lv].pages->table[sec_lv].p_index = free_frame[i];
			proc->seg_table->table[firs_lv].pages->size++;

			
			// printf("v, p: %d, %d, seg: %d \n", 
			//  	proc->seg_table->table[firs_lv].pages->table[sec_lv].v_index, 
			//  		proc->seg_table->table[firs_lv].pages->table[sec_lv].p_index,
			// 		 	proc->seg_table->size);
		}
		//printf("size: %d \n", proc->seg_table->table[0].pages->size);
		ret_mem = temp;
	}
	//mem_content(proc);
	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/*TODO: Release memory region allocated by [proc]. The first byte of
	 * this region is indicated by [address]. Task to do:
	 * 	- Set flag [proc] of physical page use by the memory block
	 * 	  back to zero to indicate that it is free.
	 * 	- Remove unused entries in segment table and page tables of
	 * 	  the process [proc].
	 * 	- Remember to use lock to protect the memory from other
	 * 	  processes.  */
	
	
	 pthread_mutex_lock(&mem_lock);
	printf("add: %d \n", address);
	int one_lv = get_first_lv(address);
	int two_lv = get_second_lv(address);
	int indexx;
	int count_size = 0;

	//for(int i = 0; i < 10; i++){
		//if(address == proc->regs[i]){
			//something happen

			indexx = proc->seg_table->table[one_lv].pages->table[two_lv].p_index;
			// clear physical mem
			while(_mem_stat[indexx].next != -1){
				_mem_stat[indexx].proc = 0;
				_mem_stat[indexx].index = 0;
				int temp = indexx;
				indexx = _mem_stat[indexx].next;
				_mem_stat[temp].next = 0;
				count_size++;
			}
			if(_mem_stat[indexx].next == -1){
				_mem_stat[indexx].proc = 0;
				_mem_stat[indexx].index = 0;
				_mem_stat[indexx].next = 0;
				count_size++;	
			}
			// clear virtual mem
			printf("FREE count_size: %d, size: %d \n", count_size, 
				proc->seg_table->table[one_lv].pages->size );
			//printf("count, two, size: %d, %d, %d \n", count_size, two_lv, proc->seg_table->table[one_lv].pages->size);
			// if(count_size + two_lv <= proc->seg_table->table[one_lv].pages->size){
			// 	int sizee = proc->seg_table->table[one_lv].pages->size;
			// 	proc->seg_table->table[one_lv].pages->size -= count_size;
			// 	printf("two: %d, c_s: %d, sizee: %d \n", two_lv, count_size, sizee);
			// 	for(int i = 0; i < sizee - two_lv - count_size; i++ ){
			// 		proc->seg_table->table[one_lv].pages->table[two_lv + i - 1].v_index = 
			// 			proc->seg_table->table[one_lv].pages->table[two_lv + i + count_size].v_index;
			// 		proc->seg_table->table[one_lv].pages->table[two_lv+i].p_index = 0;
			// 	}
			// 	// for(int i = sizee; i < 32; i++){
				// 	//proc->seg_table->table[one_lv].pages->table[two_lv + i].v_index = 0;
				// }
				
			// }
			
		//}
	//}
	pthread_mutex_unlock(&mem_lock);
	return 0;
	
}

void mem_content( struct pcb_t * proc ){
		printf("_________ALLOCATE___________\n");
	printf("	Frame used in memory: \n");
	printf("	");
	for (int i =0 ; i<NUM_PAGES; i++)
	if (_mem_stat[i].proc !=0) printf("%d  ",i);
	
	printf("\n");
	printf("	Segments used in virtual: \n");
	for (int i = 0; i<proc->seg_table->size; i++) 
	{
		printf("	+ Seg: %d\n", i);
		printf("		Pages: ");
		for (int j = 0; j<proc->seg_table->table[i].pages->size ; j++)
		printf("(%d|%d) ", proc->seg_table->table[i].pages->table[j].v_index, 
			proc->seg_table->table[i].pages->table[j].p_index);
		printf("\n\n");
	}
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	}else{
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	}else{
		return 1;
	}
}

void dump(void) {
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (	j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
}

