OVMFDIR := OVMFbin
OVMF := OVMFbin/OVMF.fd
IMAGE := Loader/bin/disk.img
BOOTLOADER := Loader/bin/bootloader.efi
KERNEL := LehaOS/bin/kernel.elf

KERNEL_MAKEFILE_DIR := LehaOS
IMAGE_MAKEFILE_DIR := Loader

run : make_kernel make_image
	qemu-system-x86_64 -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=$(OVMF) -drive file=$(IMAGE),if=ide -net none
#	qemu-system-x86_64 -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -drive file=$(IMAGE),if=ide -net none
#   qemu-system-x86_64 -drive file=$(IMAGE) -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none
make_kernel :
	$(MAKE) -C $(KERNEL_MAKEFILE_DIR) kernel

make_image :
	$(MAKE) -C $(IMAGE_MAKEFILE_DIR) image

clean:
	rm $(IMAGE) $(BOOTLOADER) $(KERNEL)
