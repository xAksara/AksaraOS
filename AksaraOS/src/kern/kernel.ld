/* Исправленный скрипт */
ENTRY(_head64)
OUTPUT_FORMAT(elf64-x86-64)

KERN_VIRT_BASE = 0xFFFFFFFF80000000;  /* Виртуальный адрес ядра */
KERN_PHYS_START = 0x100000;           /* Физический адрес загрузки */

SECTIONS {
    . = KERN_PHYS_START;
    
    .boot : {
        obj/kern/bootstrap.o (.text .data .bss)
    }

    . += KERN_VIRT_BASE;  /* Переход к виртуальным адресам */
    
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
        . = ALIGN(4K);
        bootstacktop = .;
        . += 64K;
        bootstack = .;
    }
    
    /DISCARD/ : {
        *(.comment*)
    }
}