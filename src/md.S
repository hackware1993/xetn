#if defined(__i386__)

/*
 * Internal regbuf layout
 */
#define JB_SP  0*4
#define JB_BX  1*4
#define JB_SI  2*4
#define JB_DI  3*4
#define JB_BP  4*4
#define JB_PC  5*4

        .file "md.S"
        .text

        /* setreg(regbuf env) */
.globl setreg
        .type setreg, @function
        .align 16
setreg:
        movl 4(%esp), %eax

        movl %ebx, (JB_BX)(%eax)
        movl %esi, (JB_SI)(%eax)
        movl %edi, (JB_DI)(%eax)
        /* Save SP */
        leal 4(%esp), %ecx
        movl %ecx, (JB_SP)(%eax)
        /* Save PC we are returning to */
        movl 0(%esp), %ecx
        movl %ecx, (JB_PC)(%eax)
        /* Save caller frame pointer */
        movl %ebp, (JB_BP)(%eax)
        xorl %eax, %eax
        ret
        .size setreg, .-setreg


/****************************************************************/

        /* regjmp(regbuf env, int val) */
.globl regjmp
        .type regjmp, @function
        .align 16
regjmp:
        /* First argument is jmp_buf */
        movl 4(%esp), %ecx
        /* Second argument is return value */
        movl 8(%esp), %eax
        /* Set the return address */
        movl (JB_PC)(%ecx), %edx
        /*
         * Restore registers.
         */
        movl (JB_BX)(%ecx), %ebx
        movl (JB_SI)(%ecx), %esi
        movl (JB_DI)(%ecx), %edi
        movl (JB_BP)(%ecx), %ebp
        movl (JB_SP)(%ecx), %esp
        testl %eax, %eax
        jnz  1f
        incl %eax
        /* Jump to saved PC */
1:      jmp *%edx
        .size regjmp, .-regjmp

/****************************************************************/

        /* regsw(regbuf env, int ret) */
.globl regsw
        .type regsw, @function
        .align 16
regsw:
		/* get parameter */
		movl 4(%esp), %eax
		/* exchange BX */
		movl (JB_BX)(%eax), %edx
		movl %ebx, (JB_BX)(%eax)
		movl %edx, %ebx
		/* exchange SI */
		movl (JB_SI)(%eax), %edx
		movl %esi, (JB_SI)(%eax)
		movl %edx, %esi
		/* exchange DI */
		movl (JB_DI)(%eax), %edx
		movl %edi, (JB_DI)(%eax)
		movl %edx, %edi
		/* exchange BP */
		movl (JB_BP)(%eax), %edx
		movl %ebp, (JB_BP)(%eax)
		movl %edx, %ebp
		/* PC MUST be obtained before ESP */
		/* exchange PC */
		movl (%esp), %ecx
		movl (JB_PC)(%eax), %edx
		movl %ecx, (JB_PC)(%eax)
		movl %edx, %ecx
		/* exchange SP */
		leal 4(%esp), %edx
		movl (JB_SP)(%eax), %esp
		movl %edx, (JB_SP)(%eax)
		/* return the second value */
		movl 8(%esp), %eax
		jmp *%ecx
        .size regsw, .-regsw

/****************************************************************/

#elif defined(__amd64__) || defined(__x86_64__)

/*
 * Internal regbuf layout
 */
#define JB_RSP  0*8
#define JB_RBX  1*8
#define JB_RBP  2*8
#define JB_R12  3*8
#define JB_R13  4*8
#define JB_R14  5*8
#define JB_R15  6*8
#define JB_PC   7*8

        .file "md.S"
        .text

        /* setreg(regbuf env) */
.globl setreg
        .type setreg, @function
        .align 16
setreg:
        movq %rbx, (JB_RBX)(%rdi)
        movq %rbp, (JB_RBP)(%rdi)
        movq %r12, (JB_R12)(%rdi)
        movq %r13, (JB_R13)(%rdi)
        movq %r14, (JB_R14)(%rdi)
        movq %r15, (JB_R15)(%rdi)
        /* Save SP */
        leaq 8(%rsp), %rdx
        movq %rdx, (JB_RSP)(%rdi)
        /* Save PC we are returning to */
        movq (%rsp), %rax
        movq %rax, (JB_PC)(%rdi)
        xorq %rax, %rax
        ret
        .size setreg, .-setreg


/****************************************************************/

        /* regjmp(regbuf env, int val) */
.globl regjmp
        .type regjmp, @function
        .align 16
regjmp:
        /*
         * Restore registers.
         */
        movq (JB_RBX)(%rdi), %rbx
        movq (JB_RBP)(%rdi), %rbp
        movq (JB_R12)(%rdi), %r12
        movq (JB_R13)(%rdi), %r13
        movq (JB_R14)(%rdi), %r14
        movq (JB_R15)(%rdi), %r15
        /* Set return value */
        test %esi, %esi
        mov $01, %eax
        cmove %eax, %esi
        mov %esi, %eax
        movq (JB_PC)(%rdi), %rdx
        movq (JB_RSP)(%rdi), %rsp
        /* Jump to saved PC */
        jmpq *%rdx
        .size regjmp, .-regjmp

/****************************************************************/

        /* regsw(regbuf env, int ret) */
.globl regsw
        .type regsw, @function
        .align 16
regsw:
		/* exchange RBX */
		movq (JB_RBX)(%rdi), %rdx
		movq %rbx, (JB_RBX)(%rdi)
		movq %rdx, %rbx
		/* exchange R12 */
		movq (JB_R12)(%rdi), %rdx
		movq %r12, (JB_R12)(%rdi)
		movq %rdx, %r12
		/* exchange R13 */
		movq (JB_R13)(%rdi), %rdx
		movq %r13, (JB_R13)(%rdi)
		movq %rdx, %r13
		/* exchange R14 */
		movq (JB_R14)(%rdi), %rdx
		movq %r14, (JB_R14)(%rdi)
		movq %rdx, %r14
		/* exchange R15 */
		movq (JB_R15)(%rdi), %rdx
		movq %r15, (JB_R15)(%rdi)
		movq %rdx, %r15
		/* exchange RBP */
		movq (JB_RBP)(%rdi), %rdx
		movq %rbp, (JB_RBP)(%rdi)
		movq %rdx, %rbp
		/* PC MUST be obtained before RSP */
		/* exchange PC */
		movq (%rsp), %rcx
		movq (JB_PC)(%rdi), %rdx
		movq %rcx, (JB_PC)(%rdi)
		movq %rdx, %rcx
		/* exchange RSP */
		leaq 8(%rsp), %rdx
		movq (JB_RSP)(%rdi), %rsp
		movq %rdx, (JB_RSP)(%rdi)
		/* return the second value */
		movq %rsi, %rax
		jmpq *%rcx
        .size regsw, .-regsw
#endif

