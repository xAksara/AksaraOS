#include <efi.h>
#include <efilib.h>
#include <elf.h>


EFI_STATUS
LoadFile(
	IN EFI_FILE						*Directory,
	IN CHAR16							*Path,
	IN EFI_HANDLE 				ImageHandle,
	IN EFI_SYSTEM_TABLE		*SystemTable,
	OUT EFI_FILE					*LoadedFile
	)
{
	EFI_STATUS 											Status;
	EFI_FILE												*ResultFile;
	EFI_LOADED_IMAGE_PROTOCOL				*LoadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*FileSystem;
	
	// есть протоколы и хэндлы. По сути протокол - интерфейс, для обратоки хэндлов. Когда вызываем HandleProtocol,
	// передаем хэндл, для которого хотим получить интерфейс для обработки (протокол), гуид самого протокола и 
	// указатель на указатель, куда это дело будет записываться.
	
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

	Status = Directory->Open(Directory, &ResultFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to open file. Status: %r\n", Status);
		return Status;
  }
	LoadedFile = ResultFile;
	return EFI_SUCCESS;

}

EFI_STATUS
efi_main (
	IN EFI_HANDLE 				ImageHandle,
	IN EFI_SYSTEM_TABLE 	*SystemTable
	)
{
	EFI_STATUS Status;
	EFI_FILE* KernelFile = NULL;

	InitializeLib(ImageHandle, SystemTable);
	Print(L"Hello from loader!\n\r");

	Status = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable, KernelFile);
	if (EFI_ERROR(Status)) {
		Print(L"Error: Unable to load kernel. Status: %r\n", Status);
		return Status;
  }
	else{
		Print(L"Kernel Loaded Successfully \n\r");
	}

	// Elf64

  return EFI_SUCCESS;
}