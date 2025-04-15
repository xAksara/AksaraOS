ENTRY(_head64)
OUTPUT_FORMAT(elf64-x86-64)

KERN_VIRT_BASE = 0xFFFFFFFF80000000;  
KERN_PHYS_START = 0x100000;          

SECTIONS {
    . = KERN_PHYS_START;
    
    .boot : {
        obj/kern/bootstrap.o (.text .data .bss)
    }

    . += KERN_VIRT_BASE;
    
    .text ALIGN(4K) : AT(ADDR(.text) - KERN_VIRT_BASE) {
        *(.text .text.*)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    }
    
    .data ALIGN(4K) : {
        *(.data .data.*)
    }
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss .bss.*)
    }
    
    /DISCARD/ : {
        *(.comment*)
    }
}