#include <efi.h>
#include <efilib.h>
#include <elf.h>

#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000
#define KERNEL_PHYS_START  0x100000

EFI_STATUS
LoadFile(
	IN EFI_FILE						*Directory,
	IN CHAR16							*Path,
	IN EFI_HANDLE 				ImageHandle,
	IN EFI_SYSTEM_TABLE		*SystemTable,
	OUT EFI_FILE					**LoadedFile
	)
{
	EFI_STATUS 											Status;
	// EFI_FILE												*ResultFile;
	EFI_LOADED_IMAGE_PROTOCOL				*LoadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*FileSystem;
	
	Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to get Loaded Image Protocol. Status: %r\n", Status);
		return Status;
  }
	
	Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to get Simple File System Protocol. Status: %r\n", Status);
		return Status;
  }

	if (Directory == NULL){
		Status = FileSystem->OpenVolume(FileSystem, &Directory);
		if (EFI_ERROR(Status)) {
			Print(L"Error: Unable to open root directory in a volume. Status: %r\n", Status);
			return Status;
  	}
	}

	Status = Directory->Open(Directory, LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to open file. Status: %r\n", Status);
		return Status;
  }
	return EFI_SUCCESS;

}

EFI_STATUS
ReadKernel(
	IN EFI_SYSTEM_TABLE				*SystemTable,
	IN OUT EFI_FILE_PROTOCOL	**Kernel,
	OUT UINTN									*EntryPoint
	)
{
	EFI_STATUS Status;
	UINTN Size = 0;
	EFI_FILE_INFO* FileInfo = NULL;
	Elf64_Ehdr ElfHeader;
	
	Status = (*Kernel)->GetInfo(*Kernel, &gEfiFileInfoGuid, &Size, NULL);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, Size, (void**)&FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Error: Unable to allocate pool for kernel info. Status: %r\n", Status);
			SystemTable->BootServices->FreePool(FileInfo);
			return Status;
		}
		Status = (*Kernel)->GetInfo(*Kernel, &gEfiFileInfoGuid, &Size, (void*)FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Error: Unable to get kernel info. Status: %r\n", Status);
			SystemTable->BootServices->FreePool(FileInfo);
			return Status;
		}
		Print(L"Kernel info. Name: %s, file size: %u\n", FileInfo->FileName, FileInfo->FileSize);
	} else {
		Print(L"Error: Unable to get kernel info size. Status: %r\n", Status);
		return Status;
	}

	Size = sizeof(ElfHeader);
	Status = (*Kernel)->Read(*Kernel, &Size, &ElfHeader);
	if (
		CompareMem(&ElfHeader.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		ElfHeader.e_ident[EI_CLASS] != ELFCLASS64 ||
		ElfHeader.e_ident[EI_DATA] != ELFDATA2LSB ||
		ElfHeader.e_type != ET_EXEC ||
		ElfHeader.e_machine != EM_X86_64 ||
		ElfHeader.e_version != EV_CURRENT
	)
	{
		Print(L"Error: Bad kernel ELF header.\n");
		SystemTable->BootServices->FreePool(FileInfo);
		return EFI_ABORTED;
	}
	Print(L"Kernel ELF header succefully verified.\n");
	*EntryPoint = (UINTN) ElfHeader.e_entry;

	Elf64_Phdr* Phdrs;
	Status = (*Kernel)->SetPosition(*Kernel, ElfHeader.e_phoff);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to set position to Program Headers Table. Status: %r\n", Status);
		return Status;
	}
	Size = ElfHeader.e_phentsize * ElfHeader.e_phnum;
	Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, Size, (void**)&Phdrs);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to allocate pool for program headers table. Status: %r\n", Status);
		SystemTable->BootServices->FreePool(FileInfo);
		return Status;
	}
	Status = (*Kernel)->Read(*Kernel, &Size, Phdrs);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to read Program Headers Table. Status: %r\n", Status);
		SystemTable->BootServices->FreePool(FileInfo);
		SystemTable->BootServices->FreePool(Phdrs);
		return Status;
	}

	EFI_PHYSICAL_ADDRESS MinAddress = ~0ULL;
	EFI_PHYSICAL_ADDRESS MaxAddress = 0;
	EFI_PHYSICAL_ADDRESS KernelSize = 0;

	for (Elf64_Phdr *Phdr = Phdrs; Phdr < Phdrs + ElfHeader.e_phnum; Phdr++) {
		if (Phdr->p_type == PT_LOAD) {
			Print(L"Found PT_LOAD.\n");
			MinAddress = MinAddress < Phdr->p_paddr ? MinAddress : Phdr->p_paddr;
			MaxAddress = MaxAddress > Phdr->p_paddr + Phdr->p_memsz ? MaxAddress : Phdr->p_paddr + Phdr->p_memsz;
		}
	}
	KernelSize = MaxAddress - MinAddress;
	
	Print(L"MinAddress: 0x%lx, MaxAddress: 0x%lx, KernelSize: 0x%lx\n", MinAddress, MaxAddress, KernelSize);
	Print(L"Requesting %lu pages starting at 0x%lx\n", EFI_SIZE_TO_PAGES(KernelSize), MinAddress);
	
	Status = SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(KernelSize), &MinAddress);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to allocate kernel memory. Status: %r\n", Status);
		SystemTable->BootServices->FreePool(FileInfo);
		SystemTable->BootServices->FreePool(Phdrs);
		return Status;
	}

	ZeroMem((void *)MinAddress, KernelSize);

	for (Elf64_Phdr *Phdr = Phdrs; Phdr < Phdrs + ElfHeader.e_phnum; Phdr++) {
		if (Phdr->p_type == PT_LOAD && Phdr->p_filesz > 0) {
			Status = (*Kernel)->SetPosition(*Kernel, Phdr->p_offset);
			if (EFI_ERROR(Status)) {
				Print(L"Error: Unable to set position to Program Headers Table. Status: %r\n", Status);
				SystemTable->BootServices->FreePool(FileInfo);
				SystemTable->BootServices->FreePool(Phdrs);
				SystemTable->BootServices->FreePages(MinAddress, EFI_SIZE_TO_PAGES(KernelSize));
				return Status;
			}
			Status = (*Kernel)->Read(*Kernel, &Phdr->p_filesz, (void*)Phdr->p_paddr);
			if (EFI_ERROR(Status)) {
				Print(L"Error: Unable to load file into memory. Status: %r\n", Status);
				SystemTable->BootServices->FreePool(FileInfo);
				SystemTable->BootServices->FreePool(Phdrs);
				SystemTable->BootServices->FreePages(MinAddress, EFI_SIZE_TO_PAGES(KernelSize));
				return Status;
			}
		}
	}
	return EFI_SUCCESS;
}

typedef struct {
  ///
  /// Virtual pointer to self.
  ///
  EFI_VIRTUAL_ADDRESS      SelfVirtual;

  ///
  /// UEFI services and configuration.
  ///
  EFI_VIRTUAL_ADDRESS      RTServices;                  // UEFI Runtime Services
  EFI_PHYSICAL_ADDRESS     ACPIRoot;                    // ACPI RSDP Table.

  ///
  /// Memory mapping
  ///
  UINT32                   MemoryMapDescriptorVersion;  // The memory descriptor version
  UINT64                   MemoryMapDescriptorSize;     // The size of an individual memory descriptor
  EFI_PHYSICAL_ADDRESS     MemoryMap;                   // The system memory map as an array of EFI_MEMORY_DESCRIPTOR structs
  EFI_VIRTUAL_ADDRESS      MemoryMapVirt;               // Virtual address of memmap
  UINT64                   MemoryMapSize;               // The total size of the system memory map

  ///
  /// Graphics information.
  ///
  EFI_PHYSICAL_ADDRESS     FrameBufferBase;
  UINT32                   FrameBufferSize;
  UINT32                   PixelsPerScanLine;
  UINT32                   VerticalResolution;
  UINT32                   HorizontalResolution;

	uint64_t phys_mem_start;
	uint64_t phys_mem_end;
	uint64_t kernel_phys_start;
	uint64_t kernel_phys_end;

} LOADER_PARAMS;

EFI_STATUS
AllocateMemoryMap(
	IN EFI_SYSTEM_TABLE *SystemTable,
	IN OUT LOADER_PARAMS *LoaderParams,
	OUT UINTN *MemoryMapKey
) {
	EFI_STATUS Status;
	EFI_MEMORY_DESCRIPTOR *MMap = NULL;
	UINTN MSize = 0;
	UINTN MKey = 0;
	UINTN DescriptorSize = 0;
	UINT32 DescriptorVersion = 0;

	Status = SystemTable->BootServices->GetMemoryMap(&MSize, MMap, &MKey, &DescriptorSize, &DescriptorVersion);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		MSize += 1024;
		Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, MSize, (void **)&MMap);
		if (EFI_ERROR(Status)) {
			Print(L"Error: Unable to allocate pool for Memory Map. Status: %r\n", Status);
			if (MMap != NULL) {
				SystemTable->BootServices->FreePool(MMap);
			}
			return EFI_OUT_OF_RESOURCES;
		}
		Status = SystemTable->BootServices->GetMemoryMap(&MSize, MMap, &MKey, &DescriptorSize, &DescriptorVersion);
		if (EFI_ERROR(Status)) {
			Print(L"Error: Unable to get Memory Map. Status: %r\n", Status);
			return EFI_ABORTED;
		}
		// EFI_MEMORY_DESCRIPTOR *Descriptor = MMap;
		// for (UINTN i = 0; i < MSize / DescriptorSize; i++) {
		// 		Print(L"Type: %d, PhysicalStart: 0x%lx, NumberOfPages: %lu\n",
		// 					Descriptor->Type, Descriptor->PhysicalStart, Descriptor->NumberOfPages);
		// 		Descriptor = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Descriptor + DescriptorSize);
		// }
		LoaderParams->MemoryMap = MMap;
		LoaderParams->MemoryMapSize = MSize;
		LoaderParams->MemoryMapVirt = MMap;
		LoaderParams->MemoryMapDescriptorSize = DescriptorSize;
		LoaderParams->MemoryMapDescriptorVersion = DescriptorVersion;
		*MemoryMapKey = MKey;
		return EFI_SUCCESS;
	} else {
		Print(L"Error: Unable to get Memory Map. Status: %r\n", Status);
		return EFI_ABORTED;
	}
}


typedef 
int __attribute__((sysv_abi)) 
(*KERNEL_ENTRY)(LOADER_PARAMS *);

INT64
JumpKernel(
	IN EFI_SYSTEM_TABLE				*SystemTable,
	IN EFI_HANDLE 						ImageHandle,
	IN EFI_FILE_PROTOCOL			**Kernel,
	IN UINTN 									EntryPoint
	)
{
	EFI_STATUS Status;
	LOADER_PARAMS LoaderParams;
	UINTN MemoryMapKey = 0;
	Print(L"Entry address: 0x%lx\n", EntryPoint);
	if (AllocateMemoryMap(SystemTable, &LoaderParams, &MemoryMapKey) == EFI_SUCCESS) {
		KERNEL_ENTRY KernelEntry = (KERNEL_ENTRY)(EntryPoint);
		Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MemoryMapKey);
		if (EFI_ERROR(Status)) {
			return -2;
		}
		// Print(L"Exit BS: %r\n", Status);
		(INT64)KernelEntry(&LoaderParams);
		return 1;
	} else {
		return -1;
	}
}

EFI_STATUS
efi_main (
	IN EFI_HANDLE 				ImageHandle,
	IN EFI_SYSTEM_TABLE 	*SystemTable
	)
{
	EFI_STATUS Status;
	EFI_FILE *Kernel = NULL;
	UINTN EntryPoint;

	InitializeLib(ImageHandle, SystemTable);
	Print(L"Hello from loader!\n\r");

	Status = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable, &Kernel);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to load kernel. Status: %r\n", Status);
		return EFI_ABORTED;
  }
	Print(L"Kernel Loaded Successfully \n\r");
	Status = ReadKernel(SystemTable, &Kernel, &EntryPoint);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to read kernel. Status: %r\n", Status);
		return EFI_ABORTED;
	}
	Print(L"Readed\n");
	Print(L"JUMP!!!!\n%d\n", JumpKernel(SystemTable, ImageHandle, &Kernel, EntryPoint));
	

  return EFI_SUCCESS;
}