# ProcessIsolator

Utility to hook SSDT of specific process and transfer control to a service (usermode app) for handling to determine action allow/deny API call etc. currenly only NTAPI/WIN32K logging is supposeted no handlers were implemented. Ideally this should use virtualization to hook LSTAR CSTAR MSRs and don't implement own KiSystemCall.

Requirements: 
* Win 7 SP1 x64
* PatchGuard Bypass ( http://fyyre.l2-fashion.de/ )
* Digital signing bypass ( use https://github.com/int0/ltmdm64_poc to exploit your system and clear g_CiEnabled  or use testsiging )


# Usage
* Compile
* Install driver and run svc in VM
* Drop target EXE binary in SystemDrive:\PISO
* Enjoy logged APIs

# Why
* You might want to use this to sandbox malware and collect behavioural info easy to implement because all handlers are in user-mode.
* Study code and learn kernel programming on Windows

# Example
Process Explorer is running under Process Isolator
![GitHub Logo](/images/piso.png)
