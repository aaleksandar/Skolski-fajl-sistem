// file: semaphores.h
// author: Aleksandar Abu-Samra

#pragma once

#include <windows.h>

#define SIGNAL(x) ReleaseSemaphore(x,1,NULL)
#define WAIT(x) WaitForSingleObject(x,INFINITE)

