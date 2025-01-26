.set PAGE_SHIFT, 12
.set PAGE_SIZE, 0x1000
.set KERN_STACK_SIZE, 16*PAGE_SIZE

.code64
.text

.globl entry
entry:
    # Set up kernel stack
    leaq bootstacktop(%rip),%rsp
    xor %ebp, %ebp

    # Save LoadParams in uefi_lp.
    movq %rcx, uefi_lp(%rip)

    # Invoke C code
    call kernel_main
    # Should never reach this place
    jmp .

.data

# Allocate boot stack forcing page alignment.
.p2align PAGE_SHIFT
.globl bootstack
bootstack:
.space KERN_STACK_SIZE
.globl bootstacktop
bootstacktop:


.globl uefi_lp
uefi_lp:
.quad 0
