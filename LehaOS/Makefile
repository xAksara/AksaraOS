
OSNAME := LehaOS
OVMFFIRMWARE := /usr/share/ovmf/OVMF.fd
LDS := src/kern/kernel.ld
CC := gcc
LD := ld

CFLAGS := -ffreestanding -fshort-wchar
LDFLAGS := -T $(LDS) -Bsymbolic -nostdlib -static


SRCDIR := src
OBJDIR := obj
KERN_SRCDIR := $(SRCDIR)/kern
KERN_OBJDIR := $(OBJDIR)/kern
BINDIR := bin

KERN_SRCFILES = $(KERN_SRCDIR)/kernel.c
KERN_OBJFILES := $(KERN_OBJDIR)/kernel.o

kernel: $(KERN_OBJFILES) link


$(KERN_OBJDIR)/%.o: $(KERN_SRCDIR)/%.c
	@echo !======== COMPILING
	$(CC) $(CFLAGS) -c $< -o $@

link:
	@echo !======== LINKING
	$(LD) $(LDFLAGS) -o $(BINDIR)/kernel.elf $(KERN_OBJFILES)
