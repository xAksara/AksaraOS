LOADER := bootloader
IMAGE := disk

SRCDIR := src
OBJDIR := obj
BINDIR := bin


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


$(OBJDIR)/$(LOADER).so : $(OBJDIR)/$(LOADER).o
	ld $(OBJDIR)/$(LOADER).o                       \
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




$(OBJDIR)/$(LOADER).o : $(SRCDIR)/$(LOADER).c
	gcc $(SRCDIR)/$(LOADER).c           \
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
      -o $(OBJDIR)/$(LOADER).o


image: loader $(BINDIR)/startup.nsh
	dd if=/dev/zero of=$(BINDIR)/$(IMAGE).img bs=512 count=93750
	mformat -i $(BINDIR)/$(IMAGE).img
	mmd -i $(BINDIR)/$(IMAGE).img ::/EFI
	mmd -i $(BINDIR)/$(IMAGE).img ::/EFI/BOOT
	mcopy -i $(BINDIR)/$(IMAGE).img $(BINDIR)/$(LOADER).efi ::/EFI/BOOT
	mcopy -i $(BINDIR)/$(IMAGE).img $(BINDIR)/startup.nsh ::
