# activate-desktop-ini

<img src="misc/icon.png" width=128px/>

Cloned repos don't show my nice folder icons?
Download `activate-desktop-ini.exe` and drag the folder onto it.

## usage

![](misc/usage.png)

Alternatively:

```shell
$ git clone https://github.com/coalpha/repo cloned-repo
$ activate-desktop-ini cloned-repo
```

## why

When I clone my git repos, there's a `desktop.ini` file in there.
This is what *should* configure the folder icon to work correctly.
However, that's not enough to make it work properly. There are two conditions:

1. The folder containing desktop.ini needs to have the readonly attribute set.
2. The desktop.ini file must have the hidden and system attributes set

Now there's a catch. Windows explorer caches things and so you need to nudge it
into realizing that it needs to refresh that specific folder's icon.

## methods

Updating the attributes is simple, the hard part is trying to get explorer to
actually update the icon.

### COM + IFileOperation

1. Move `desktop.ini` to `desktop.ini.temp`
2. Change file attributes
3. Use Shell APIs to move `desktop.ini.temp` to `desktop.ini`

The directory will display the default icon and can be coaxed into displaying
the correct one on refresh/f5.

### SHChangeNotify

1. Change file attributes
2. `SHChangeNotify` with one of the following:
   - `SHCNE_ALLEVENTS`
   - `SHCNE_ATTRIBUTES`
   - `SHCNE_CREATE`
   - `SHCNE_DELETE`
   - `SHCNE_MKDIR`
   - `SHCNE_ALLEVENTS`
   - `SHCNE_RENAMEITEM`

Inconsistent results, almost never changes the icon, even on refresh.

### SHGetSetFolderCustomSettings

1. Change file attributes
2. Use `SHGetSetFolderCustomSettings` to `FCS_READ` settings.
3. Use `SHGetSetFolderCustomSettings` to `FCS_FORCEWRITE` settings.
