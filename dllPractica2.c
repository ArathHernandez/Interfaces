/***********************************************************************\
*                                                                       *
* InpOut32drv.cpp                                                       *
*                                                                       *
* The entry point for the InpOut DLL                                    *
* Provides the 32 and 64bit implementation of InpOut32 DLL to install   *
* and call the appropriate driver and write directly to hardware ports. *
*                                                                       *
* Written by Phillip Gibbons (Highrez.co.uk)                            *
* Based on orriginal, written by Logix4U (www.logix4u.net)              *
*                                                                       *
\***********************************************************************/

#include <Windows.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>

int inst64();
int start(LPCTSTR pszDriver);

//First, lets set the DRIVERNAME depending on our configuraiton.
#define DRIVERNAMEx64 _T("inpoutx64\0")

#define IOCTL_READ_PORT_UCHAR	 -1673519100 //CTL_CODE(40000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_PORT_UCHAR	 -1673519096 //CTL_CODE(40000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define S_Busy 0x80
#define S_ACK 0x40
#define S_Paper_End 0x20
#define S_Select_In 0x10
#define S_nerror 0x08


char str[10];

HANDLE hdriver = NULL;
HINSTANCE hmodule;
SECURITY_ATTRIBUTES sa;

int Opendriver();
void Closedriver(void);

BOOL APIENTRY DllMain(HINSTANCE  hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	hmodule = hModule;
	switch (ul_reason_for_call)	{
		case DLL_PROCESS_ATTACH:
			Opendriver();
			break;
		case DLL_PROCESS_DETACH:
			Closedriver();
			break;
	}
	return TRUE;
}

/***********************************************************************/

void Closedriver(void) {
	if (hdriver) {
		CloseHandle(hdriver);
		hdriver = NULL;
	}
}

__declspec(dllexport) void _stdcall Out32(short PortAddress, short data) {
	unsigned int error;
	DWORD BytesReturned;
	BYTE Buffer[3] = {0,0,0};
	unsigned short* pBuffer;
	pBuffer = (unsigned short *)&Buffer[0];
	*pBuffer = LOWORD(PortAddress);
	Buffer[2] = LOBYTE(data);

	error = DeviceIoControl(hdriver,IOCTL_WRITE_PORT_UCHAR,&Buffer,3,NULL,0,&BytesReturned,NULL);
}

/*********************************************************************/

__declspec(dllexport) short _stdcall Inp32(short PortAddress) {
	unsigned int error;
	DWORD BytesReturned;
	UCHAR ucRes;
	unsigned char Buffer[3] = {0,0,0};
	unsigned short * pBuffer;
	pBuffer = (unsigned short *)&Buffer;
	*pBuffer = LOWORD(PortAddress);
	Buffer[2] = 0;
	error = DeviceIoControl(hdriver,IOCTL_READ_PORT_UCHAR,&Buffer,2,&Buffer,1,&BytesReturned,NULL);

	ucRes = (UCHAR)Buffer[0];
	return ucRes;
}

/*********************************************************************/

int Opendriver() {
	hdriver = CreateFile("\\\\.\\inpoutx64",GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hdriver == INVALID_HANDLE_VALUE) {
		hdriver = NULL;
		return 0;
	}
	return 1;
}

/*********************************************************************/

__declspec(dllexport) int _stdcall IsInpOutDriverOpen() {
	if (hdriver != INVALID_HANDLE_VALUE && hdriver != NULL) {
			return 1;
	}
	return 0;
}
/*********************************************************************/
__declspec(dllexport) void send(short p_base, short data){
	unsigned char aux;
	unsigned char preControl;
	unsigned char digits[16]={0x3F,0x06,0xDA,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x4F,0x77,0x7C,0x38,0x5E,0x79,0x71};
	unsigned char MASK_CTL=0x0B;
	aux=Inp32(p_base+2)^MASK_CTL;
	preControl=aux&0xFE;
	Out32(p_base+2,preControl^MASK_CTL);
	Sleep(100);
	data>15?Out32(p_base,0x40):Out32(p_base,digits[data]);
	Sleep(100);
	preControl=aux|1;
	Out32(p_base+2,preControl^MASK_CTL);
}
/*********************************************************************/

__declspec(dllexport) short get(short p_base){
	unsigned char mask_status[]={S_Busy,S_Paper_End,S_Select_In,S_nerror,
						 S_Busy,S_Paper_End,S_Select_In,S_nerror};
	unsigned char status;
	unsigned char MASK_CTL=0x0B;
	unsigned char MASK_STS=0x80;
	unsigned char data=0;
	unsigned int i;
	unsigned int aux=1;
	unsigned int control;
	for(i=0;i<8;i++,aux<<=1){
		if(i==0){
			control=Inp32(p_base+2)^MASK_CTL;
			control=control&0xF7;
			Out32(p_base+2,control^MASK_CTL);
			Sleep(100);
		}
		else if(i==4){
			control=Inp32(p_base+2)^MASK_CTL;
			control=control&0x08;
			Out32(p_base+2,control^MASK_CTL);
			Sleep(100);
		}
		status=Inp32(p_base+1)^MASK_STS;
		if(status&mask_status[i]!=0){
			data=data|aux;
		}
	}
	return(data);

}
