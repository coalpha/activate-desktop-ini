#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>
#include <shellapi.h>

#define DWORD_PTR *(DWORD *)
#define QWORD_PTR *(unsigned long long *)

int start(void) {
   PWSTR dir;
   size_t dir_len;
   {
      int argc;
      PCWSTR arg1 = CommandLineToArgvW(GetCommandLineW(), &argc)[1];

      if (argc != 2) {
         #define usage L"activate_desktop_ini.exe directory\n"
         WriteConsoleW(
            GetStdHandle(STD_OUTPUT_HANDLE),
            usage,
            sizeof(usage) / sizeof(WCHAR),
            NULL,
            NULL
         );
         return 1;
      }

      DWORD cwd_len = GetCurrentDirectoryW(0, NULL);

      size_t arg1_len = 0;
      while (arg1[arg1_len++]);
      // Both arg1_len and cwd_len include their null bytes.
      // cwd_len's null byte is going to be used for the path separator, though.
      dir_len = cwd_len + arg1_len;

      dir = _alloca(dir_len * sizeof(WCHAR));
      GetCurrentDirectoryW(cwd_len, dir);

      PWSTR dir_wh = dir + cwd_len - 1;
      *dir_wh++ = L'\\';
      while (*arg1) *dir_wh++ = *arg1++;
      *dir_wh = L'\0';
   }

   {
      DWORD const attr = GetFileAttributesW(dir);
      if (attr == INVALID_FILE_ATTRIBUTES) {
         return 1;
      }

      SetFileAttributesW(dir, attr | FILE_ATTRIBUTE_READONLY);
   }

   #define mDesktopIni L"/desktop.ini"
   PWSTR desktop_ini =
      _alloca(dir_len * sizeof(WCHAR) + sizeof(mDesktopIni));

   #define mDesktopIniTmp L"/desktop.ini.tmp"
   PWSTR desktop_ini_tmp =
      _alloca(dir_len * sizeof(WCHAR) + sizeof(mDesktopIniTmp));

   {
      PCWSTR dir_rh = dir;
      PWSTR desktop_ini_wh = desktop_ini;
      PWSTR desktop_ini_tmp_wh = desktop_ini_tmp;

      while (*dir_rh) *desktop_ini_wh++ = *desktop_ini_tmp_wh++ = *dir_rh++;

      QWORD_PTR desktop_ini_wh = QWORD_PTR desktop_ini_tmp_wh = QWORD_PTR L"\\des"; desktop_ini_wh += 4; desktop_ini_tmp_wh += 4;
      QWORD_PTR desktop_ini_wh = QWORD_PTR desktop_ini_tmp_wh = QWORD_PTR L"ktop" ; desktop_ini_wh += 4; desktop_ini_tmp_wh += 4;
      QWORD_PTR desktop_ini_wh = QWORD_PTR desktop_ini_tmp_wh = QWORD_PTR L".ini" ; desktop_ini_wh += 4; desktop_ini_tmp_wh += 4;
      QWORD_PTR desktop_ini_tmp_wh = QWORD_PTR L".tmp"; desktop_ini_tmp_wh += 4;
      *desktop_ini_wh = *desktop_ini_tmp_wh = L'\0';
   }

   if (!MoveFileW(desktop_ini, desktop_ini_tmp)) {
      return GetLastError();
   }

   {
      DWORD const attr = GetFileAttributesW(desktop_ini_tmp);
      if (attr == INVALID_FILE_ATTRIBUTES) {
         return GetLastError();
      }

      // attrib +s +h $dir/desktop.ini.tmp
      if (!SetFileAttributesW(desktop_ini_tmp, 0
         | attr
         | FILE_ATTRIBUTE_HIDDEN
         | FILE_ATTRIBUTE_SYSTEM
      )) {
         return GetLastError();
      }
   }

   {
      HRESULT const res = CoInitializeEx(NULL, 0
         | COINIT_DISABLE_OLE1DDE
         | COINIT_SPEED_OVER_MEMORY
      );
      if (res < 0) {
         return res;
      }
   }

   IFileOperation *ifo;
   {
      HRESULT const res = CoCreateInstance(
         &CLSID_FileOperation,
         NULL,
         CLSCTX_ALL,
         &IID_IFileOperation,
         (void **) &ifo
      );
      if (res < 0) {
         return res;
      }
   }

   // {
   //    HRESULT const res = ifo->lpVtbl->SetOperationFlags(ifo, FOF_NO_UI);
   //    if (res < 0) {
   //       return res;
   //    }
   // }

   IShellItem *iFrom;
   {
      HRESULT const res = SHCreateItemFromParsingName(
         L"C:\\test",
         NULL,
         &IID_IShellItem,
         (void **) &iFrom
      );
      if (res < 0) {
         return res;
      }
   }

   IShellItem *iDir;
   {
      HRESULT const res = SHCreateItemFromParsingName(
         dir,
         NULL,
         &IID_IShellItem,
         (void **) &iDir
      );
      if (res < 0) {
         return res;
      }
   }

   {
      HRESULT const res = ifo->lpVtbl->MoveItem(
         ifo,
         iFrom,
         iDir,
         L"desktop.ini",
         NULL
      );
   }
   CoUninitialize();

   return 0;
}
