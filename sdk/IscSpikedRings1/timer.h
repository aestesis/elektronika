//-----------------------------------------------------------------------------
// CTimer
//
// This class can be used to keep track of time in your applications. It also
// measures the framerate if Frame() is called once per frame.
//
// Requires winmm.lib when compiled
//
// Copyright (c) 2000 Andreas Jönsson
//-----------------------------------------------------------------------------
#ifndef TIMER_H
#define TIMER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define FPS_INTERVAL 0.5f

class CTimer
{
public:
	CTimer();

	void  Start();
	void  Stop();
	void  Continue();

 	float GetTime();
	float GetFrameTime();
	float GetDeltaTime();

	void  Frame();
	float GetFps();

	bool  IsRunning();

protected:
	bool   m_bRunning;	

	UINT64 m_BaseTicks;
	UINT64 m_StopTicks;

	UINT64 m_TicksPerSecond;

	float  m_fFrameTime;
	float  m_fDeltaTime;

	DWORD  m_dwNumFrames;
	float  m_fLastUpdate;
	float  m_fFps;
};

#endif