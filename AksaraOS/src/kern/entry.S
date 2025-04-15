.set PAGE_SHIFT, 12
.set PAGE_SIZE, 0x1000
.set KERN_STACK_SIZE, 16*PAGE_SIZE

.code64
.text

.globl entry
entry:
    leaq bootstacktop(%rip),%rsp
    xor %ebp, %ebp

    movq %rdi, LoaderParams(%rip)

    call kernel_main
    jmp .

.data

.p2align PAGE_SHIFT
.globl bootstack
bootstack:
.space KERN_STACK_SIZE
.globl bootstacktop
bootstacktop:


.globl LoaderParams
LoaderParams:
.quad 0
