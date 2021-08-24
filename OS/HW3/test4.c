#include <stdio.h>
#include "rmalloc.h"

int 
main ()
{
	void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;

	rmprint() ;

	p1 = rmalloc(2000) ; 
	printf("p1 rmalloc(9000):%p\n", p1) ; 

	p2 = rmalloc(2000) ; 
	printf("p2 rmalloc(2500):%p\n", p2) ; 

	// rfree(p1) ; 
	// printf("p1 rfree(%p)\n", p1) ; 
	// rmprint() ;
	
	p3 = rmalloc(2000) ; 
	printf("p3 rmalloc(1000):%p\n", p3) ; 

	p4 = rmalloc(2000) ; 
	printf("p4 rmalloc(1000):%p\n", p4) ; 
	rmprint() ;

	p4 = rrealloc(p4, 3000);
	printf("p4 rralloc p4 1000 -> 3000:%p\n", p4) ; 
	rmprint() ;

	rfree(p1);
	printf("p1 rfree(%p)\n", p2);
	rmprint() ;
	rfree(p3);
	printf("p3 rfree(%p)\n", p2);
	rmprint() ;
	rfree(p2);
	printf("p2 rfree(%p)\n", p2);
	rmprint() ;
	rfree(p4);
	printf("p1 rfree(%p)\n", p2);
	rmprint() ;
	rmshrink();
	printf("after rmshrink()\n");
	rmprint();
}