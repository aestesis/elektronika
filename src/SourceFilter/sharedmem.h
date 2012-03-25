#pragma once

#include			<windows.h>


#define				SHAREDMEMBODYSIZE		(320*240*3)

typedef struct
{
	int				frame;
	volatile int	locked;
	BYTE			body[SHAREDMEMBODYSIZE];
} Tsharedmem;

#define				SHAREDMEMSIZE			sizeof(Tsharedmem)
#define				SHAREDMEMNAME			"SharedElektroSource"

class Asharedmem
{
public:
									Asharedmem();
	virtual							~Asharedmem();

	HANDLE							map;
	Tsharedmem						*data;
	bool							thefirst;
};