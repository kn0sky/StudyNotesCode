#include "pch.h"
#include "FastMutex.h"

void FastMutex::init()
{
	ExInitializeFastMutex(&_FastMutex);
}

void FastMutex::Lock()
{
	ExAcquireFastMutex(&_FastMutex);
}

void FastMutex::UnLock()
{
	ExReleaseFastMutex(&_FastMutex);
}