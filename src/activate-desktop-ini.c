#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>
#include <shellapi.h>

#define true ((_Bool) 1)
#define false ((_Bool) 0)

#define DWORD_PTR *(DWORD *)
#define QWORD unsigned long long
#define QWORD_PTR *(QWORD *)

#define usage L"activate-desktop-ini.exe directory\n"
#define usage_len (sizeof(usage) / sizeof(wchar_t) - 1)

/// COM Error Handling
static inline void CoEH(HRESULT const res) {
   if (__builtin_expect(res < 0, false)) {
      ExitProcess(res);
      __builtin_unreachable();
   }
}

void start(void) {
   int argc;
   wchar_t const *const restrict dir =
      CommandLineToArgvW(GetCommandLineW(), &argc)[1];

   if (__builtin_expect(argc, 2) != 2) {
      HANDLE const stderr = GetStdHandle(STD_ERROR_HANDLE);
      WriteConsoleW(stderr, usage, usage_len, (DWORD[]){0}, NULL);
      ExitProcess(1);
      __builtin_unreachable();
   }

   _Bool failure;
   DWORD attr;

   failure = !SetCurrentDirectoryW(dir);
   if (__builtin_expect(failure, false))
      goto BAD_END_WINDOWS;

   attr = GetFileAttributesW(L".");
   if (attr == INVALID_FILE_ATTRIBUTES)
      goto BAD_END_WINDOWS;

   failure = !SetFileAttributesW(L".", attr | FILE_ATTRIBUTE_READONLY);
   if (__builtin_expect(failure, false))
      goto BAD_END_WINDOWS;

   // First, we're gonna move desktop.ini to desktop.ini.tmp.
   // Then we're going to change it's attributes.
   // Once that's done, we're going to use shell APIs via COM to move it back to
   // desktop.ini.
   // This is enough to tell explorer to invalidate the cached icon and refetch.

   failure = !MoveFileW(L"desktop.ini", L"desktop.ini.tmp");
   if (__builtin_expect(failure, false))
      goto BAD_END_WINDOWS;

   attr = GetFileAttributesW(L"desktop.ini.tmp");
   if (__builtin_expect(attr == INVALID_FILE_ATTRIBUTES, false))
      goto BAD_END_WINDOWS;

   failure = !SetFileAttributesW(L"desktop.ini.tmp", 0
      | attr
      | FILE_ATTRIBUTE_HIDDEN
      | FILE_ATTRIBUTE_SYSTEM
   );
   if (__builtin_expect(failure, false))
      goto BAD_END_WINDOWS;

   CoEH(CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY));

   // var shFileOp = new FileOperation();
   IFileOperation *const restrict shFileOp;
   CoEH(CoCreateInstance(&CLSID_FileOperation, NULL, CLSCTX_ALL, &IID_IFileOperation, (void **) &shFileOp));

   // shFileOp.showGUI = false;
   CoEH(shFileOp->lpVtbl->SetOperationFlags(shFileOp, FOF_NO_UI));

   // var shFrom = new ShellPath("desktop.ini.tmp");
   IShellItem *const restrict shFrom;
   CoEH(SHCreateItemFromParsingName(L"desktop.ini.tmp", NULL, &IID_IShellItem, (void **) &shFrom));

   // ver shTo = new ShellPath("desktop.ini");
   IShellItem *const restrict shTo;
   CoEH(SHCreateItemFromParsingName(dir, NULL, &IID_IShellItem, (void **) &shTo));

   // shFileOp.MoveItem(shFrom, shTo);
   CoEH(shFileOp->lpVtbl->MoveItem(shFileOp, shFrom, shTo, L"desktop.ini", NULL));
   CoEH(shFileOp->lpVtbl->PerformOperations(shFileOp));

   // delete shFileOp;
   // delete shFrom;
   // delete shTo;
   shFileOp->lpVtbl->Release(shFileOp);
   shFrom->lpVtbl->Release(shFrom);
   shTo->lpVtbl->Release(shTo);

   CoUninitialize();

   ExitProcess(0);
   __builtin_unreachable();

   BAD_END_WINDOWS:
   ExitProcess(GetLastError());
   __builtin_unreachable();
}
