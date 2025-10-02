@echo off
echo Starting server and client in Windows Terminal...
set BASE_PATH=%~dp0../..
wt -w 0 ^
    nt --title "SERVER" cmd /k "cd /d %BASE_PATH%/server && npm run dev" ^
    ; nt --title "CLIENT" cmd /k "cd /d %BASE_PATH%/client && npm run dev"
echo All processes started in Windows Terminal!


:: FOR BETTER EXPERIENCE WHEN RUNNING THE APPLICATION :
:: - press Window + R and type "cmd" then enter
:: - right click on the navigation bar -> setting
:: - click "Open json file" in the left bottom corner
:: - find "profiles" and then in the line [   "defaults": {},   ] then replace that line with [   "defaults": {"suppressApplicationTitle": true},   ]
:: - save and quit


:: FOR RUNNING THE SYSTEM :
:: - Open window powershell and type ".\run.bat" or open cmd and type "run.bat" to run the system


:: THIS FILE MUST BE REMOVED BEFORE DEPLOYMENT