//////////////////////////////////////////////////////////////////////////
/// 
/// File:           pi.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator shared header file
/// 
//////////////////////////////////////////////////////////////////////////
#pragma once
#ifdef _PI_KRN                  /// use PI in windows kernel space
#include <ntifs.h>
#include <ntstrsafe.h>
#elif _PI_USR                   /// otherwise use PI in windows user space
#include <windows.h>
#include <strsafe.h>
#include <winternl.h>
#else
#error "project environment is not specified"
#endif

#define NAMESPACE_PI_BEGIN namespace PI {
#define NAMESPACE_PI_END }
#define PIPOOL 'osIP'   //PIso

//
// shared includes
//
#include <log/pi_log.h>
#include <mem/pi_memory.h>
#include <thread/pi_thread.h>
#include <lpc/nt_lpc.h>
#include <lpc/pi_lpc_client.h>
#include <lpc/pi_lpc_server.h>

#include "pi_types.h"
#include "pi_ioctl.h"



