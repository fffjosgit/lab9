
.code32           
.globl tmp_entry
tmp_entry:

    movw $0x5307, %ax 
	movw $0x0003, %cx 
	movw $0x0001, %bx
	int $0x15  