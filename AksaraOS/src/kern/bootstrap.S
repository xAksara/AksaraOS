.code64

.set PAGE_SHIFT, 12
.set HUGE_PAGE_SHIFT, PAGE_SHIFT + 9
.set PAGE_SIZE, 0x1000 # 4KB
.set HUGE_PAGE_SIZE, 0x200000 # 2MB


.set pml4,   pml4phys
.set pdpt1,  pml4 + PAGE_SIZE
.set pdpt2,  pml4 + 2*PAGE_SIZE
.set pde1,   pml4 + 3*PAGE_SIZE
.set pde2,   pml4 + 4*PAGE_SIZE

# 4096/8 * 11
.set PML_SIZE, 5632


.set PDP_ENTRY_COUNT, 512
.set PTE_P, 0x001
.set PTE_W, 0x002
.set PTE_PS, 0x080

.text
.globl _head64
_head64:
    cli

    # параметр из загрузчика
    movq %rcx, %r12

    # обнуляем память под таблицы страницы
    leaq pml4(%rip), %rdi
    xorl %eax, %eax
    movl $PML_SIZE, %ecx
    rep stosq

 
    # Kern base virt: 0b1111111111111111 111111111 111111110 000000000 000000000 000000000000
    # ##############                        511       510        0         0           0
    
    # PML4[0] → PDPT1 (0b0 - 0b111111111 111111111 111111111 111111111111 = 512ГБ)
    leaq pml4(%rip), %rdi
    leaq (pdpt1 + (PTE_P | PTE_W))(%rip), %rax
    movq %rax, (%rdi)                           # Запись в PML4[0]

    # PML4[511] → PDPT2 (KERN_VIRT_BASE: 0xFFFFFFFF80000000) (последние 512Гб)
    #
    leaq (pdpt2 + (PTE_P | PTE_W))(%rip), %rax
    movq %rax, 0xFF8(%rdi)                      # PML4[511] (смещение 511*8=4088=0xFF8)



    # PDPT1[0] -> PDE1 (0b0 - 0b111111111 111111111 111111111111 = 1ГБ)
    leaq pdpt1(%rip), %rdi
    leaq (pde1 + (PTE_P | PTE_W))(%rip), %rax
    movq %rax, (%rdi)   # Запись в PDPT1[0]

    # PDPT2[510] -> PDE2 (первый гигабайт, начиная с вирт адреса ядра)
    leaq pdpt2(%rip), %rdi
    leaq (pde2 + (PTE_P | PTE_W))(%rip), %rax
    movq %rax, 0xFF0(%rdi)   # Запись в PDPT2[1] (0xFF0 = 510*8)



    # PDE1[i] = MBZ (512 страниц по 2 МБ)
    movq $-PDP_ENTRY_COUNT, %rcx
    leaq pde1(%rip), %rdi
    leaq pde2(%rip), %rsi
    movl $(PTE_P|PTE_W|PTE_PS), %eax
1:
    movq %rax, PAGE_SIZE(%rsi,%rcx,8)
    movq %rax, PAGE_SIZE(%rdi,%rcx,8)
    addq $HUGE_PAGE_SIZE, %rax
    incq %rcx
    jnz 1b
    
    leaq pml4(%rip), %rax
    movq %rax, %cr3
   
    movabs $entry, %rax
    jmpq *%rax

    
.data

.p2align PAGE_SHIFT
.globl pml4phys
pml4phys:
.space 11*PAGE_SIZE

.globl pdefreestart
.set pdefreestart, pde2 + PAGE_SIZE

.globl pdefreeend
.set pdefreeend, pml4phys + 11*PAGE_SIZE
