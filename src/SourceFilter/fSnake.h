//------------------------------------------------------------------------------
// File: fSnake.h
//
// Desc: define CSnakeFilter class
//
// Author : Ashok Jaiswal
//
// Data/Time : September 2004
//------------------------------------------------------------------------------

#if !defined(FSNAKE_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define FSNAKE_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSnakeStream;


class CSnakeFilter : public CSource
{
	friend class CSnakeStream;

public:
	CSnakeFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CSnakeFilter(void);

// static
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

// Attributes
public:

// Operations
public:

// Overrides
protected:

// Implementations
protected:
	// IUnknown interface
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}


// member variables
private:
};


#endif // !defined(FSNAKE_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
