
#include "windows.h"
#include "stdio.h"

typedef void(__stdcall *lpOut32)(short, short);
typedef short(__stdcall *lpInp32)(short);
typedef BOOL(__stdcall *lpIsInpOutDriverOpen)(void);
typedef BOOL(__stdcall *lpIsXP64Bit)(void);

//Apuntadores a rutinas del DLL Ãºtiles para acceso al "Device Driver"
lpOut32 gfpOut32;
lpInp32 gfpInp32;
lpIsInpOutDriverOpen gfpIsInpOutDriverOpen;
lpIsXP64Bit gfpIsXP64Bit;

void theBeep(unsigned int freq) {
	gfpOut32(0x43, 0xB6);
	gfpOut32(0x42, (freq & 0xFF));
	gfpOut32(0x42, (freq >> 9));
	Sleep(10);
	gfpOut32(0x61, gfpInp32(0x61) | 0x03);
}

void StopTheBeep() {
	gfpOut32(0x61, (gfpInp32(0x61) & 0xFC));
}

int main(int argc, char* argv[]) {
	if (argc<3)	{
		//Enviar un mensaje de error al usuario, cuando no ha introducido los parÃ¡metros correctos
		printf("Error : faltan parametros\n\n***** Uso correcto *****\n\nmyReadWritePort read <ADDRESS> \no \nmyReadWritePort write <ADDRESS> <DATA>\n\n\n\n\n");
	}
	else {
		//Cargar el DLL a nuestro espacio de memoria
		HINSTANCE hInpOutDll;
		hInpOutDll = LoadLibrary("D:\\Ricardo\\ITESM\\IEC\\PuertoParalelo\\Practica1\\Debug\\InpOutx64.DLL");	//Ruta y nombre del DLL
		if (hInpOutDll != NULL)	{
			gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "send");
			gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "get");
			gfpIsInpOutDriverOpen = (lpIsInpOutDriverOpen)GetProcAddress(hInpOutDll, "IsInpOutDriverOpen");
			gfpIsXP64Bit = (lpIsXP64Bit)GetProcAddress(hInpOutDll, "IsXP64Bit");

			if (gfpIsInpOutDriverOpen()) {
				//Indicar con sonidos que se ha logrado el acceso al DDL y al "device driver"
				theBeep(2000);
				Sleep(200);
				theBeep(1000);
				Sleep(300);
				theBeep(2000);
				Sleep(250);
				StopTheBeep();

				if (!strcmp(argv[1], "read")) {
					short iPort = atoi(argv[2]);
					WORD wData = gfpInp32(iPort);	//Leer el puerto
					printf("Dato leido del puerto %s ==> %d \n\n\n\n", argv[2], wData);
				}
				else if (!strcmp(argv[1], "write"))	{
					if (argc<4)	{
						printf("Error en los argumentos ingresados");
						printf("\n***** Uso correcto *****\n\nmyReadWritePort read <ADDRESS> \no \nmyReadWritePort write <ADDRESS> <DATA>\n\n\n\n\n");
					}
					else {
						short iPort = atoi(argv[2]);
						WORD wData = atoi(argv[3]);
						gfpOut32(iPort, wData);
						printf("data written to %s\n\n\n", argv[2]);
					}
				}
			}
			else {
				printf("Unable to open InpOutx64 Driver!\n");
			}

			//Fin del programa ...
			FreeLibrary(hInpOutDll);	//.. descargar DLL
			return 0;
		}
		else
		{
			printf("No puede encontrar el DLL InpOutx64!!! xP\n");
			return -1;
		}
	}
	return -2;
}
