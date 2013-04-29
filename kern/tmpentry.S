


.code32           
.globl tmp_entry
tmp_entry:

    movw $5307h, %ax 
	movw $0003h, %cx 
	movw $0001h, %bx
	int $0x15  