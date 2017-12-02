#pragma once
#include <windows.h>

// PTEB __forceinline NtCurrentTeb()
// {
// #ifdef _AMD64_
// 	return (PTEB)__readgsqword(FIELD_OFFSET(NT_TIB, Self));
// #elif _X86_
// 	return (PTEB)__readfsdword(FIELD_OFFSET(NT_TIB, Self));
// #endif
// 	return NULL;
// }