#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "rmalloc.h" 

//next와 size가 들어가있음 (next, size), 하나의 노드는 16bytes사용함

rm_header rm_free_list = { 0x0, 0 } ;
rm_header rm_used_list = { 0x0, 0 } ;
void * address[100];
size_t size[100];
int page_count = 0;
rm_option policy = 2;
int max_page = 1;


static 
void * 
_data (rm_header_ptr e)
{
	return ((void *) e) + sizeof(rm_header) ;
}

static
void rsplit (rm_header_ptr pre_hole, rm_header_ptr hole, size_t s)
{
	rm_header_ptr remainder = (rm_header_ptr) (_data(hole) + s), itr = 0x0, temp = &rm_used_list;
	remainder->size = hole->size - s - sizeof(rm_header);
	remainder->next = hole->next;
	pre_hole->next = remainder;
	hole->size = s;
	hole->next = rm_used_list.next;
	rm_used_list.next = hole;
}

void * rmalloc (size_t s) 
{
	// TODO
	rm_used_list.size += s;
	if(s <= 0){
		printf("wrong size.\n");
		return 0x0;
	}
	rm_header_ptr hole = 0x0, itr = 0x0, temp = &rm_free_list, pre_hole = &rm_free_list;
	int best = max_page * (int) getpagesize();
	int worst = -1;
	itr = rm_free_list.next;
	for(itr = rm_free_list.next; itr != 0x0; itr = itr->next){
		if(policy == BestFit){
			if((int)(itr->size - s - sizeof(rm_header)) <=  best && (int)(itr->size - s - sizeof(rm_header)) >= 0){
				best = (int)(itr->size - s - sizeof(rm_header));
				pre_hole = temp;
				hole = itr;
			}
		}
		else if(policy == WorstFit){
			if((int)(itr->size - s - sizeof(rm_header)) >= worst){
				worst = (int)(itr->size - s - sizeof(rm_header));
				printf("check worst : %d\n", worst);
				pre_hole = temp;
				hole = itr;
			}
		}
		else{//FirstFit
			if( (int)(s <= itr->size - sizeof(rm_header)) ){
				hole = itr;
				break;
			}
			pre_hole = itr;
		}
		temp = itr;
	}
	//적당한 크가가 없을 때
	if(hole == 0x0){
		size_t pagesize = getpagesize();
		size_t need_page = (s + sizeof(rm_header)) / pagesize + 1;
		if(max_page < (int) need_page)
			max_page = (int) need_page;
		size_t mmap_size = need_page * pagesize;
		if(page_count == 101){
			perror("rheap overflow. :)\n");
			exit(1);
		}
		hole = mmap(NULL, mmap_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);
		page_count++;
		address[page_count - 1] = hole;
		size[page_count - 1] = mmap_size - sizeof(rm_header);
		if(hole == 0x0)
			return 0x0;
		hole->size = mmap_size - sizeof(rm_header);
		hole->next = itr;
		temp->next = hole;
		pre_hole = temp;
	}

	if(s < hole->size)
		rsplit(pre_hole, hole, s);
	else if(s == hole->size){
		// free list 정리
		pre_hole->next = hole->next;
		// used list 정리
		hole->next = rm_used_list.next;
		rm_used_list.next = hole;
	}
	
	return _data(hole);
}


void rfree (void * p) 
{
	// TODO 
	//p가 잘못된 값이면 return 0x0
	//used를 free보내는거
	rm_header_ptr itr = 0x0, hole = 0x0, temp = &rm_used_list;
	// p가 있으면 break
	for(itr = rm_used_list.next; itr != 0x0; itr = itr->next){
		if(p == _data(itr)){ // used를 free로 보내기
			hole = itr;
			break;
		}
		temp = itr;
	}
	// p가 없으면 멈춤
	if(hole == 0x0){
		perror("rsegementation fault. (there is nothing or wrong address)\n");
		exit(1);
	}
	// used_list정리
	temp->next = hole->next;
	rm_used_list.size -= hole->size;
	// coalesce
	int i;
	for(i = 0; i < page_count; i++){
		if((rm_header_ptr)address[i] <= hole && hole < (rm_header_ptr)address[i] + size[i] + sizeof(rm_header))
			break;
	}
	temp = &rm_free_list;
	for(itr = rm_free_list.next; itr != 0x0; itr = itr->next){
		// 전, 후 free
		if((rm_header_ptr) (_data(temp) + temp->size) == hole && (rm_header_ptr) (_data(hole) + hole->size) == itr){
			temp->size += sizeof(rm_header) + hole->size + sizeof(rm_header) + itr->size;
	 		temp->next = itr->next;
			//printf("check1\n");
			return ;
		}
		// 전 free
		else if((rm_header_ptr) (_data(temp) + temp->size) == hole && (rm_header_ptr) (_data(hole) + hole->size) != itr){
			temp->size += sizeof(rm_header) + hole->size;
			//printf("check2\n");
			return ;
		}
		// 후 free
		else if((rm_header_ptr) (_data(temp) + temp->size) != hole && (rm_header_ptr) (_data(hole) + hole->size) == itr){
			//printf("check3\n");
			hole->size += sizeof(rm_header) + itr->size;
			hole->next = itr->next;
			temp->next = hole;
			return ;
		}
		else if((rm_header_ptr) (_data(temp) + temp->size) != hole && (rm_header_ptr) (_data(hole) + hole->size) != itr){
			//temp 와 itr가 한 영역에 같이 있을 때
			if((rm_header_ptr)address[i] <= temp && temp < (rm_header_ptr)(_data(address[i]) + size[i]) && (rm_header_ptr)address[i] < itr && itr < (rm_header_ptr)(_data(address[i]) + size[i])){
				//temp hole itr 일때
				if(temp < hole && hole < itr){
					hole->next = itr;
					temp->next = hole;
					//printf("check4\n");
					return ;
				}
			}
			//hole과 itr가 한 영역에 같이 있을 때
			else if(!((rm_header_ptr)address[i] <= temp && temp < (rm_header_ptr)(_data(address[i]) + size[i])) && (rm_header_ptr)address[i] < itr && itr < (rm_header_ptr)(_data(address[i]) + size[i])){
				//hole itr순일 때
				if((rm_header_ptr) (_data(hole) + hole->size) < itr){
					hole->next = itr;
					temp->next = hole;
					//printf("check5\n");
					return ;
				}
			}//temp와 hole이 한 영역에 있을 때 ????어렵다
			else if((rm_header_ptr)address[i] <= temp && temp < (rm_header_ptr)(_data(address[i]) + size[i]) && !((rm_header_ptr)address[i] < itr && itr < (rm_header_ptr)(_data(address[i]) + size[i]))){
				//temp->next가 범위에 없으면
				if(!((rm_header_ptr)address[i] <= temp->next && temp->next < (rm_header_ptr)(_data(address[i]) + size[i]))){
					hole->next = itr;
					temp->next = hole;
					//printf("check6\n");
					return ;
				}
			}
		}
		temp = itr;
	}
	//끝 부분을 할 떄
	if(itr == 0x0){
		//printf("check7\n");
		hole->next = itr;
		temp->next = hole;
		return;
	}
	//제일 처음 부분을 할 떄
	//printf("check8\n");
	hole->next = rm_free_list.next;
	rm_free_list.next = hole;
}

void * rrealloc (void * p, size_t s) 
{
	// TODO
	if((int)s < 0)
		return 0x0;
	else if((int)s == 0){
		rfree(p);
		return 0x0;
	}
	else if((int)s > 0){
		void * new_p;
		rm_header_ptr itr = 0x0, new = 0x0, temp = &rm_used_list;
		// p가 있으면 break
		for(itr = rm_used_list.next; itr != 0x0; itr = itr->next){
			if(p == _data(itr)){
				new = itr;
				break;
			}
			temp = itr;
		}
		// p가 없으면 멈춤
		if(new == 0x0){
			perror("rrealloc rsegementation fault. (there is nothing or wrong address)\n");
			exit(1);
		}

		new_p = rmalloc(s);
		if(new_p == 0x0){
			return 0x0;
		}
		size_t old_size = new->size;
		size_t new_size = s;
		memcpy(new_p, p, old_size < new_size ? old_size : new_size);
		rfree(p);
		return new_p;
	}
}

void rmshrink () 
{
	// TODO
	for(int i = 0; i < page_count; i++){
		rm_header_ptr itr = 0x0, temp = &rm_free_list;
		for(itr = rm_free_list.next; itr != 0x0; itr = itr->next){
			if(address[i] == itr && size[i] == itr->size){
				temp->next = itr->next;
				munmap(address[i], size[i]);
				break;
			}
			temp = itr;
		}
	}
}

void rmconfig (rm_option opt) 
{
	policy = opt;
}


void 
rmprint () 
{
	rm_header_ptr itr ;
	int i ;

	printf("==================== rm_free_list ====================\n") ;
	for (itr = rm_free_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

	printf("==================== rm_used_list ====================\n") ;
	for (itr = rm_used_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}