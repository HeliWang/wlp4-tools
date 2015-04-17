; Binary Buddy (best fit) memory allocator written in CS241-MIPS
; Copyright Sean Harrap (2015)
; Design (new/delete/init behaviour and names) based off
; Brad Lushman's allocator for compatibility reasons.
; Written following CS241's style guidelines for MIPS code.
; No error-checking on passed pointers, so something terrible
; might happen if delete is fed invalid input.

;;Global variables
;Set manually
heapSize:	.word 0x20000 ;Heap size in bytes      (divisible by 4)

;Calculated automatically
heapPosn:	.word codeEnd

;;Public functions
.export init
.export new
.export delete

;Initializes the heap: Sets up the free list and heap positions
;based on the array length passed in $2, and sets up the only block.
;Preserves all registers.
init:		sw $1, -4($30)
		sw $3, -8($30)
		lis $3
		.word -8
		add $30, $30, $3

		;Set up heap position
		lis $3
		.word heapPosn
		lw $1, 0($3)
		add $1, $1, $2
		add $1, $1, $2
		add $1, $1, $2
		add $1, $1, $2
		sw $1, 0($3)

		;Set up the freelist:
		;Size: Size of heap
		;Next: NULL
		lis $3
		.word heapSize
		sw $3, 0($1)
		sw $0, 4($1)

		;Cleanup
		lis $3
		.word 8
		add $30, $30, $3
		lw $3, -8($30)
		lw $1, -4($30)
		jr $31

;Inserts a block into the free list
freeListInsert:	sw $1, -4($30)

		lis $2
		.word heapPosn
		;TODO


fliEnd:		lw $1, -4($30)

;Gets heap memory.
;$1 = Amount of memory to get (in words)
;$3 = Pointer to the memory received (0 upon failure)
new:		sw $1, -4($30)
		sw $2, -8($30)
		sw $4, -12($30)
		sw $5, -16($30)
		sw $6, -20($30)
		sw $7, -24($30)
		sw $31, -28($30)
		lis $31
		.word -28
		add $30, $30, $31

		;Minimum block size: 4 words
		;(Will be automatically doubled once)
		lis $3
		.word 2

		;Determine how much memory to allocate
		;Note that this will automatically reserve a
		;byte for allocation information
newLoop1S:	add $3, $3, $3
		slt $6, $1, $3
		beq $6, $0, newLoop1S

		;Find the best candidate in the free list
		;$1 = current best candidate
		;$2 = size of candidate
		lis $5
		.word heapPosn
		add $1, $0, $0
		lis $2
		.word 0xFFFFFFFF

		beq $5, $0, newError
newLoop2S:	lw $4, 0($5)

		;If the current candidate is smaller than the best...
		slt $6, $4, $2
		beq $6, $0, 4
		;And big enough to accomodate the requested memory...
		slt $6, $4, $3
		bne $6, $0, 2
		;Update the best candidate
		add $1, $5, $0
		add $2, $4, $0

		lw $5, 4($5)
		bne $5, $0, newLoop2S

		;Now that the ideal candidate has been found, split it until its size is ideal.
		beq $1, $0, newError
		lis $5
		.word 2
		add $7, $1, $0

newLoop3S:	divu $2, $5
		mflo $4
		slt $6, $4, $3
		bne $6, $0, newLoop3E
		;Insert the newly split block into the free list
		mflo $2
		sw $2, 0($7)
		add $1, $2, $7
		lis $6
		.word freeListInsert
		jalr $6
		beq $0, $0, newLoop3S

		;Successfully allocated a block.
		;Check if we need to update the heap pointer
		lis $6
		.word heapPosn
		lw $4, 0($6)
		bne $7, $4, 2
		lw $7, 4($7)
		sw $7, 0($6)

		;Fix block address and return it
newLoop3E:	add $5, $5, $5
		add $3, $7, $5

newCleanup:	lis $31
		.word 28
		add $30, $30, $31
		lw $31, -28($30)
		lw $7, -24($30)
		lw $6, -20($30)
		lw $5, -16($30)
		lw $4, -12($30)
		lw $2, -8($30)
		lw $1, -4($30)
		jr $31

newError:	add $3, $0, $0
		beq $0, $0, newCleanup

delete:		sw $1, -4($30)
		sw $2, -8($30)
		sw $31, -12($30)
		lis $31
		.word -12
		add $30, $30, $31

		lis $2
		.word 4
		sub $1, $1, $2
		lis $2
		.word freeListInsert
		jalr $2

deleteCleanup:	lis $31
		.word 12
		add $30, $30, $31
		lw $31, -12($30)
		lw $2, -8($30)
		lw $1, -4($30)
		jr $31

codeEnd:
