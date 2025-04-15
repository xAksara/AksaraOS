#include <efi.h>

typedef struct {
  EFI_VIRTUAL_ADDRESS      SelfVirtual;


  EFI_VIRTUAL_ADDRESS      RTServices;                  // UEFI Runtime Services
  EFI_PHYSICAL_ADDRESS     ACPIRoot;                    // ACPI RSDP Table.


  UINT32                   MemoryMapDescriptorVersion;  // The memory descriptor version
  UINT64                   MemoryMapDescriptorSize;     // The size of an individual memory descriptor
  EFI_PHYSICAL_ADDRESS     MemoryMap;                   // The system memory map as an array of EFI_MEMORY_DESCRIPTOR structs
  EFI_VIRTUAL_ADDRESS      MemoryMapVirt;               // Virtual address of memmap
  UINT64                   MemoryMapSize;               // The total size of the system memory map


  EFI_PHYSICAL_ADDRESS     FrameBufferBase;
  UINT32                   FrameBufferSize;
  UINT32                   PixelsPerScanLine;
  UINT32                   VerticalResolution;
  UINT32                   HorizontalResolution;
} LOADER_PARAMS;


extern void _head64();  // Точка входа
extern char bootstacktop[];  // Стек
extern LOADER_PARAMS *LoaderParams;

void kernel_main(LOADER_PARAMS *LoaderParams) {
    __asm__ volatile("mov %0, %%rax" : : "r"(0x111ULL));
    __asm__ volatile("mov %0, %%rbx" : : "r"(0x111ULL));
    unsigned y = 50;
    unsigned BBP = 4;
    uint32_t temp = LoaderParams->HorizontalResolution;
    for(unsigned x = 0; x < LoaderParams->HorizontalResolution / 2 * BBP; ++x) {
      // unsigned* pix = (unsigned *)((x + y * LoaderParams->PixelsPerScanLine) * BBP + LoaderParams->FrameBufferBase);
      // *pix = 0xffffffff;
			*(unsigned int*)(x + (y*LoaderParams->PixelsPerScanLine*BBP)+LoaderParams->FrameBufferBase) = 0xffffffff;
    }
    for (;;);
}