@echo off
setlocal

echo Dang xoa file sysnetdef.db...
del "..\..\server\database\sysnetdef.db"

if exist "..\..\server\database\sysnetdef.db" (
    echo Loi: Khong the xoa file sysnetdef.db
    exit /b
)

echo Dang sao chep sysnetdef-backup.db...
copy "..\..\server\database\sysnetdef-backup.db" "..\..\server\database\sysnetdef.db"

if exist "..\..\server\database\sysnetdef.db" (
    echo Thanh cong: Da xoa va thay the file sysnetdef.db
) else (
    echo Loi: Khong the sao chep file sysnetdef-backup.db
)

endlocal
