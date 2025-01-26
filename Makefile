OVMFDIR := OVMFbin
OVMF := OVMFbin/OVMF.fd
IMAGE_OBJDIR = Loader/obj
IMAGE := Loader/bin/disk.img
BOOTLOADER := Loader/bin/bootloader.efi
KERNEL_OBJDIR = AksaraOS/obj/kern
KERNEL := AksaraOS/bin/kernel.elf


KERNEL_MAKEFILE_DIR := AksaraOS
IMAGE_MAKEFILE_DIR := Loader

run : make_kernel make_image
#	qemu-system-x86_64 -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=$(OVMF) -drive file=$(IMAGE),if=ide -net none
	qemu-system-x86_64 -s -S -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=$(OVMF) -drive file=$(IMAGE),if=ide -net none

make_kernel :
	$(MAKE) -C $(KERNEL_MAKEFILE_DIR) kernel

make_image :
	$(MAKE) -C $(IMAGE_MAKEFILE_DIR) image

clean:
	rm -f $(IMAGE) $(BOOTLOADER) $(KERNEL) $(IMAGE_OBJDIR)/*.o $(KERNEL_OBJDIR)/*.o 
