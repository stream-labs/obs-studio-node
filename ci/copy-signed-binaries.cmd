appveyor DownloadFile ^
	"https://github.com/obsproject/obs-studio/releases/download/%OBSVersion%/OBS-Studio-%OBSVersion%-Full-x64.zip"

7z x "OBS-Studio-%OBSVersion%-Full-x64.zip" -oobs-studio-jp9000

robocopy ^
	"obs-studio-jp9000\data\obs-plugins\win-capture" ^
	"%SLFullDistributePath%\obs-studio-node\libobs\data\obs-plugins\win-capture" & exit 0