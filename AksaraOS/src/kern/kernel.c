extern void _head64();  // Точка входа
extern char bootstacktop[];  // Стек

typedef struct {
  ///
  /// Virtual pointer to self.
  ///
  unsigned long long      SelfVirtual;

  ///
  /// UEFI services and configuration.
  ///
  unsigned long long      RTServices;                  // UEFI Runtime Services
  unsigned long long     ACPIRoot;                    // ACPI RSDP Table.

  ///
  /// Memory mapping
  ///
  unsigned long                   MemoryMapDescriptorVersion;  // The memory descriptor version
  unsigned long long                   MemoryMapDescriptorSize;     // The size of an individual memory descriptor
  unsigned long long     MemoryMap;                   // The system memory map as an array of EFI_MEMORY_DESCRIPTOR structs
  unsigned long long      MemoryMapVirt;               // Virtual address of memmap
  unsigned long long                   MemoryMapSize;               // The total size of the system memory map

  ///
  /// Graphics information.
  ///
  unsigned long long     FrameBufferBase;
  unsigned long                   FrameBufferSize;
  unsigned long                   PixelsPerScanLine;
  unsigned long                   VerticalResolution;
  unsigned long                   HorizontalResolution;

} LOADER_PARAMS;


int kernel_main(LOADER_PARAMS *LoaderParams) {
    __asm__ volatile("mov %0, %%rax" : : "r"(0x111ULL));
    __asm__ volatile("mov %0, %%rbx" : : "r"(0x111ULL));
    for (;;);
}