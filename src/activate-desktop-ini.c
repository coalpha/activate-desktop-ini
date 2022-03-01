#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <shlobj_core.h>
#include <shellapi.h>

#define DWORD_PTR *(DWORD *)
#define QWORD unsigned long long
#define QWORD_PTR *(QWORD *)

#define true ((_Bool) 1)
#define false ((_Bool) 0)

#define usage L"activate-desktop-ini.exe directory\n"
#define usage_len (sizeof(usage) / sizeof(wchar_t) - 1)
void start(void) {
   HANDLE const stderr = GetStdHandle(STD_ERROR_HANDLE);
   {
      int argc;
      wchar_t const *const restrict dir =
         CommandLineToArgvW(GetCommandLineW(), &argc)[1];

      if (__builtin_expect(argc, 2) != 2) {
         WriteConsoleW(stderr, usage, usage_len, (DWORD[]){0}, NULL);
         ExitProcess(1);
         __builtin_unreachable();
      }

      _Bool const failure = !SetCurrentDirectoryW(dir);
      if (__builtin_expect(failure, false))
         goto BAD_END_WINDOWS;
   }

   {
      DWORD const attr = GetFileAttributesW(L".");
      if (__builtin_expect(attr == INVALID_FILE_ATTRIBUTES, false))
         goto BAD_END_WINDOWS;

      _Bool const failure = !SetFileAttributesW(L".", attr | FILE_ATTRIBUTE_READONLY);
      if (__builtin_expect(failure, false))
         goto BAD_END_WINDOWS;
   }

   DWORD dir_len = GetCurrentDirectoryW(0, NULL);
   wchar_t *const restrict dir = __builtin_alloca(dir_len * sizeof(wchar_t));

   dir_len = GetCurrentDirectoryW(dir_len, dir);
   if (__builtin_expect(dir_len == 0, false))
      goto BAD_END_WINDOWS;

   {
      DWORD const attr = GetFileAttributesW(L"desktop.ini");
      if (__builtin_expect(attr == INVALID_FILE_ATTRIBUTES, false))
         goto BAD_END_WINDOWS;

      _Bool const failure = !SetFileAttributesW(L"desktop.ini", 0
         | attr
         | FILE_ATTRIBUTE_HIDDEN
         | FILE_ATTRIBUTE_SYSTEM
      );

      if (__builtin_expect(failure, false))
         goto BAD_END_WINDOWS;
   }

   // SHFOLDERCUSTOMSETTINGS s;
   // SHGetSetFolderCustomSettings(&s, L".", FCS_READ);
   // SHGetSetFolderCustomSettings(&s, L".", FCS_FORCEWRITE);
   SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_PATH | SHCNF_FLUSH, L".", NULL);

   ExitProcess(0);
   __builtin_unreachable();

   BAD_END_WINDOWS:
   ExitProcess(GetLastError());
   __builtin_unreachable();
}
