LOADER := bootloader
IMAGE := disk
SRCDIR := src
OBJDIR := obj
BINDIR := bin

KERNEL_MAKEFILE_DIR := ../AksaraOS
KERNEL_PATH := ../AksaraOS/bin/kernel.elf


SRCFILES = $(wildcard $(SRCDIR)/*.c)
OBJFILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCFILES))

loader: $(BINDIR)/$(LOADER).efi

$(BINDIR)/$(LOADER).efi : $(OBJDIR)/$(LOADER).so
	objcopy -j .text              \
          -j .sdata                 \
          -j .data                  \
          -j .dynamic               \
          -j .dynsym                \
          -j .rel                   \
          -j .rela                  \
          -j .reloc                 \
          --target=efi-app-x86_64   \
          $(OBJDIR)/$(LOADER).so    \
          $(BINDIR)/$(LOADER).efi


$(OBJDIR)/$(LOADER).so : $(OBJFILES)
	ld $(OBJFILES)                \
     /usr/lib/crt0-efi-x86_64.o     \
     -nostdlib                      \
     -znocombreloc                  \
     -T /usr/lib/elf_x86_64_efi.lds \
     -shared                        \
     -Bsymbolic                     \
     -L /usr/lib                    \
     -l:libgnuefi.a                 \
     -l:libefi.a                    \
     -o $(OBJDIR)/$(LOADER).so




$(OBJDIR)/%.o : $(SRCDIR)/%.c
	gcc $<           \
      -c                                  \
      -fno-stack-protector                \
      -ffreestanding                      \
      -fpic                               \
      -fshort-wchar                       \
      -mno-red-zone                       \
      -I /usr/include/efi        		\
      -I /usr/include/efi/x86_64/ 	      \
      -DEFI_FUNCTION_WRAPPER              \
      -DGNU_EFI_USE_MS_ABI                \
      -o $@


make_kernel:
	$(MAKE) -C $(KERNEL_MAKEFILE_DIR) kernel

image: loader make_kernel $(BINDIR)/startup.nsh
	dd if=/dev/zero of=$(BINDIR)/$(IMAGE).img bs=512 count=93750
	mformat -i $(BINDIR)/$(IMAGE).img
	mmd -i $(BINDIR)/$(IMAGE).img ::/EFI
	mmd -i $(BINDIR)/$(IMAGE).img ::/EFI/BOOT
	mcopy -i $(BINDIR)/$(IMAGE).img $(BINDIR)/$(LOADER).efi ::/EFI/BOOT
	mcopy -i $(BINDIR)/$(IMAGE).img $(BINDIR)/startup.nsh ::
	mcopy -i $(BINDIR)/$(IMAGE).img $(KERNEL_PATH) ::