OSNAME := AksaraOS
OVMFFIRMWARE := /usr/share/ovmf/OVMF.fd
LDS := src/kern/kernel.ld
CC := gcc
AS := as
LD := ld

CFLAGS := -ffreestanding -fshort-wchar -g -nostdlib -mcmodel=large \
					-I /usr/include/efi        		\
      		-I /usr/include/efi/x86_64/
					
LDFLAGS := -T $(LDS) -Bsymbolic -nostdlib -static -g -z max-page-size=4096
ASFLAGS := --64 -Iinc/ -g

SRCDIR := src
OBJDIR := obj
KERN_SRCDIR := $(SRCDIR)/kern
KERN_OBJDIR := $(OBJDIR)/kern
BINDIR := bin

KERN_SRCFILES := $(KERN_SRCDIR)/bootstrap.S \
								 $(KERN_SRCDIR)/entry.S			\
								 $(KERN_SRCDIR)/kernel.c

KERN_OBJFILES := $(patsubst $(KERN_SRCDIR)/%.c,$(KERN_OBJDIR)/%.o,$(filter %.c,$(KERN_SRCFILES)))
KERN_OBJFILES += $(patsubst $(KERN_SRCDIR)/%.S,$(KERN_OBJDIR)/%.o,$(filter %.S,$(KERN_SRCFILES)))

.PHONY: all kernel link clean

all: kernel

kernel: $(KERN_OBJFILES) link

$(KERN_OBJDIR)/%.o: $(KERN_SRCDIR)/%.c
	@echo "!======== COMPILING $<"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -Iinc/ -c $< -o $@

$(KERN_OBJDIR)/%.o: $(KERN_SRCDIR)/%.S
	@echo "!======== ASSEMBLING $<"
#	@mkdir -p $(@D)
#	$(CC) -D__ASSEMBLER__ -E -P -Iinc/ $< -o $(@D)/$*.i  # Препроцессор -> временный файл
	$(AS) $(ASFLAGS) -o $@ $< 

link:
	@echo "!======== LINKING"
	@mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $(BINDIR)/kernel.elf $(KERN_OBJFILES)

clean:
	rm -rf $(OBJDIR) $(BINDIR)