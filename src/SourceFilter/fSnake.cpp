//------------------------------------------------------------------------------
// File: fSnake.cpp
//
// Desc: implement CSnakeFilter class
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Snake.h"
#include "fSnake.h"
#include "fSnake_op.h"

//
// Constructor
//
CSnakeFilter::CSnakeFilter(LPUNKNOWN pUnk, HRESULT *phr)
	: CSource(NAME("CElektronikaSource"), pUnk, CLSID_Snake)
{
    CAutoLock cAutoLock(&m_cStateLock);
	
	//
	// CSource::AddPin() is automatically called
	// by creating a source stream (CSourceStream)
	//

	// Add one source stream (output pin)!
	new CSnakeStream(phr, this, L"Capturer", pinCAPTURE);

	new CSnakeStream(phr, this, L"Aperçu", pinPREVIEW);

	// TODO: add source stream here
}


//
// Destructor
//
CSnakeFilter::~CSnakeFilter()
{
}


