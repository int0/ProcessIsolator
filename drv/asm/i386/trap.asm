.686p
.model FLAT, C
option casemap :none


ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

public PI_FastCallEntry
public NT_FastCallEntry

LIST_ENTRY struct
	Flink	dd	?
	Blink	dd	?
LIST_ENTRY ends


PI_PROCESS_ENTRY struct
	List		LIST_ENTRY	<?>
	ProcessId	dd			?
PI_PROCESS_ENTRY ends

PI_PROCESS_LIST struct
	Head		LIST_ENTRY	<?>
	SpinLock	dd			?
PI_PROCESS_LIST ends

.FARDATA

externdef PsGetCurrentProcessId:proc
externdef PI_KiFastCallEntry_Handler:proc
externdef g_PIProcessList:PI_PROCESS_LIST


.data

NT_FastCallEntry dd 0

.code

PI_FastCallEntry proc

	jmp		NT_FastCallEntry
;pushad
;pushfd
;push fs
;push ds
;
;push 30h
;pop fs
;
;mov ecx, 23h
;mov ds, ecx
;mov es, ecx
;
;
;
;pop ds
;pop fs
;popfd
;popad
;
;push 0
;ret
PI_FastCallEntry endp

end