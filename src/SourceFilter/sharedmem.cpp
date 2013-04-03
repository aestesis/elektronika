#include		"sharedmem.h"



Asharedmem::Asharedmem()
{
	thefirst=FALSE;
	map=NULL;
	data=NULL;
	map = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHAREDMEMSIZE, SHAREDMEMNAME);
	if(map&&!(GetLastError()==ERROR_ALREADY_EXISTS))
		thefirst=true;
	else
		thefirst=false;
	if(map)
	{
		data = (Tsharedmem *)MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, SHAREDMEMSIZE);
		if(thefirst)
			data->frame=1;
	}
}

Asharedmem::~Asharedmem()
{
	if(data)
	{
		UnmapViewOfFile(data);
		data=NULL;
	}
	if(map)
	{
		CloseHandle(map); 
		map = NULL; 
	} 
}

