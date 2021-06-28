#include <windows.h>

#define usage L"activate_desktop_ini.exe directory\n"

int WINAPI mainCRTStartup() {
   wchar_t *any;
   wchar_t *path = CommandLineToArgvW(GetCommandLineW(), (int *) &any)[1];

   if ((int) any != 2) {
      WriteConsoleW(
         GetStdHandle(STD_OUTPUT_HANDLE),
         usage,
         sizeof(usage) / sizeof(wchar_t),
         NULL,
         NULL
      );
      return 1;
   }

   any = (wchar_t *) GetFileAttributesW(path);
   if ((DWORD) any == INVALID_FILE_ATTRIBUTES) {
      return 1;
   }

   SetFileAttributesW(
      path, 0
      | (DWORD) any
      | FILE_ATTRIBUTE_READONLY
   );

   for (any = path; *any; ++any);

   __builtin_memcpy(any, L"/desktop.ini", sizeof(L"/desktop.ini"));

   any = (wchar_t *) GetFileAttributesW(path);
   if ((DWORD) any == INVALID_FILE_ATTRIBUTES) {
      return 1;
   }

   SetFileAttributesW(
      path, 0
      | (DWORD) any
      | FILE_ATTRIBUTE_HIDDEN
      | FILE_ATTRIBUTE_SYSTEM
   );

   return 0;
}
