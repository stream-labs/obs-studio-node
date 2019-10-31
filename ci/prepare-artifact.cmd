if [%OBSVersion:~-1%] EQU [0] (
    curl -kLO "https://github.com/obsproject/obs-studio/releases/download/%SignedOBSVersion%/OBS-Studio-%SignedOBSVersion:.0=%-Full-x64.zip" -f --retry 5
    7z x "OBS-Studio-%OBSVersion:.0=%-Full-x64.zip" -oobs-studio-jp9000
) else (
    curl -kLO "https://github.com/obsproject/obs-studio/releases/download/%SignedOBSVersion%/OBS-Studio-%SignedOBSVersion%-Full-x64.zip" -f --retry 5
    7z x "OBS-Studio-%OBSVersion%-Full-x64.zip" -oobs-studio-jp9000
)

curl -kL "https://s3.amazonaws.com/getsentry-builds/getsentry/breakpad-tools/windows/breakpad-tools-windows.zip" -o "breakpadtools.zip" -f --retry 5
7z x "breakpadtools.zip" > nul
    
curl -kL "https://github.com/getsentry/sentry-cli/releases/download/1.48.0/sentry-cli-Windows-i686.exe" -o "sentry-cli.exe" -f --retry 5
    
robocopy "obs-studio-jp9000\data\obs-plugins\win-capture" "%SLFullDistributePath%\obs-studio-node\data\obs-plugins\win-capture" & exit 0