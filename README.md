# OpenFile++
OpenFile++ is a simple file opener.  
The installer adds it to the Windows Startup group so that it runs automatically on login.  
It is hidden by default.  
Add your commonly used files to the list for quick access to them, especially if they are in disparate directories.

## Usage
- Press Ctrl+Shift+O or click the program icon in the taskbar tray to open the window.
- By default, the list is populated with files in your Documents, Downloads, and Desktop directories (but not sub-directories by default).
- Start typing to find a file in the list.  You can enter multiple terms separated by a space.  If only a single item is left in the list, press enter to open it in the associated application.
- Up/down/page-up/page-down  change selection in the list even while focus is in the filter edit field.
- Press enter to open the item(s) selected item in the list.
- Press Ctrl+N to add individual files to the list.
- Press Ctrl+D to add the contents of other directories to the list.
- Edit the list (Edit | Edit source list) to remove or deactivate items, or to make directory searches include sub-directories.

## Development Notes
- Dependent upon Qt5
- Open openfileplusplus/project/openfilepp.pro in Qt Creator to build
- In Visual Studio 2019, open openfileplusplus as a folder (openfileplusplus/project/CppProperties.json is dependent upon QTDIR environment variable being set similar to QTDIR=D:\Qt\5.15.0\msvc2019\ )
- Windows installer built using Inno Setup (see readme in openfileplusplus/setup/)
