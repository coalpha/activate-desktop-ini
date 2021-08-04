/* I'm just gonna copy my discord chat logs here.
[12:11 PM] coalpha: when I clone my git repos, there's a desktop.ini in there
[12:11 PM] coalpha: which are what configure the folder icon to work correctly
[12:13 PM] coalpha: however, that's not enough to make it work properly
[12:13 PM] coalpha: the folder containing desktop.ini needs to have the readonly
                    attribute set
[12:14 PM] coalpha: secondly, the desktop.ini file must have the hidden and
                    system attributes set
[12:15 PM] coalpha: at this point, all of the conditions have been fulfilled for
                    windows explorer to properly render the folder icon
[12:15 PM] coalpha: now there's a catch
[12:15 PM] coalpha: windows explorer caches things
[12:16 PM] coalpha: and so you need to nudge it into realizing that it needs to
                    refresh that specific folder's icon
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>
#include <shellapi.h>

#define DWORD_PTR *(DWORD *)
#define QWORD unsigned long long
#define QWORD_PTR *(unsigned long long *)

void start(void) {
   {
      int argc;
      wchar_t const *const restrict dir =
         CommandLineToArgvW(GetCommandLineW(), &argc)[1];

      if (argc != 2) {
         #define usage L"activate_desktop_ini.exe directory\n"
         WriteConsoleW(
            GetStdHandle(STD_OUTPUT_HANDLE),
            usage,
            sizeof(usage) / sizeof(WCHAR) - 1,
            NULL,
            NULL
         );
         goto BAD_END_WINDOWS;
      }

      if (!SetCurrentDirectoryW(dir)) {
         goto BAD_END_WINDOWS;
      }
   }

   {
      DWORD const attr = GetFileAttributesW(L".");
      if (attr == INVALID_FILE_ATTRIBUTES) {
         goto BAD_END_WINDOWS;
      }

      if (!SetFileAttributesW(L".", attr | FILE_ATTRIBUTE_READONLY)) {
         goto BAD_END_WINDOWS;
      }
   }

   DWORD dir_len = GetCurrentDirectoryW(0, NULL);
   wchar_t *const restrict dir =
      __builtin_alloca(dir_len * sizeof(wchar_t));
   if (dir == NULL) {
      goto BAD_END_WINDOWS;
   }

   if (!GetCurrentDirectoryW(dir_len, dir)) {
      goto BAD_END_WINDOWS;
   }

   DWORD const desktop_ini_len = GetFullPathNameW(L"desktop.ini", 0, NULL, NULL);
   wchar_t *const restrict desktop_ini = __builtin_alloca(desktop_ini_len * sizeof(wchar_t));
   if (!GetFullPathNameW(L"desktop.ini", desktop_ini_len, desktop_ini, NULL)) {
      goto BAD_END_WINDOWS;
   }

   #define mDesktopTmp L"desktop.ini.tmp"
   DWORD desktop_ini_tmp_len = GetFullPathNameW(mDesktopTmp, 0, NULL, NULL);
   wchar_t *const restrict desktop_ini_tmp = __builtin_alloca(desktop_ini_tmp_len * sizeof(WCHAR));
   if (!GetFullPathNameW(mDesktopTmp, desktop_ini_tmp_len, desktop_ini_tmp, NULL)) {
      goto BAD_END_WINDOWS;
   }

   // First, we're gonna move desktop.ini to desktop.ini.tmp.
   // Then we're going to change it's attributes.
   // Once that's done, we're going to use shell APIs via COM to move it back to
   // desktop.ini.
   // This is enough to tell explorer to invalidate the cached icon and refetch.

   if (!MoveFileW(desktop_ini, desktop_ini_tmp)) {
      goto BAD_END_WINDOWS;
   }

   {
      DWORD const attr = GetFileAttributesW(desktop_ini_tmp);
      if (attr == INVALID_FILE_ATTRIBUTES) {
         goto BAD_END_WINDOWS;
      }

      if (!SetFileAttributesW(desktop_ini_tmp, 0
         | attr
         | FILE_ATTRIBUTE_HIDDEN
         | FILE_ATTRIBUTE_SYSTEM
      )) {
         goto BAD_END_WINDOWS;
      }
   }

   {
      HRESULT const res = CoInitializeEx(NULL, 0
         | COINIT_DISABLE_OLE1DDE
         | COINIT_SPEED_OVER_MEMORY
      );
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   IFileOperation *const restrict file_op;
   {
      HRESULT const res = CoCreateInstance(
         &CLSID_FileOperation,
         NULL,
         CLSCTX_ALL,
         &IID_IFileOperation,
         (void **) &file_op
      );
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   {
      HRESULT const res = file_op->lpVtbl->SetOperationFlags(file_op, FOF_NO_UI);
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   IShellItem *const restrict shFrom;
   {
      HRESULT const res = SHCreateItemFromParsingName(
         desktop_ini_tmp,
         NULL,
         &IID_IShellItem,
         (void **) &shFrom
      );
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   IShellItem *const restrict shTo;
   {
      HRESULT const res = SHCreateItemFromParsingName(
         dir,
         NULL,
         &IID_IShellItem,
         (void **) &shTo
      );
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   {
      HRESULT const res = file_op->lpVtbl->MoveItem(
         file_op,
         shFrom,
         shTo,
         L"desktop.ini",
         NULL
      );
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   {
      HRESULT const res = file_op->lpVtbl->PerformOperations(file_op);
      if (res < 0) {
         ExitProcess(res);
         __builtin_unreachable();
      }
   }

   file_op->lpVtbl->Release(file_op);
   shFrom->lpVtbl->Release(shFrom);
   shTo->lpVtbl->Release(shTo);

   CoUninitialize();

   ExitProcess(0);
   __builtin_unreachable();

   BAD_END_WINDOWS:
   ExitProcess(GetLastError());
   __builtin_unreachable();
}
