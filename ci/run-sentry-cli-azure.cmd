rem -> copy all .pdb files from the root dir
for /f %%f in ('dir /b "%SLFullDistributePath%\obs-studio-node\*.pdb"') do "%RootDirectory%\dump_syms.exe" %%f > "%RootDirectory%\syms\%%~nf.sym"

rem -> copy all .pdb files from plugins dir
for /f %%f in ('dir /b "%SLFullDistributePath%\obs-studio-node\streamlabs-build\distribute\obs-studio-node\obs-plugins\64bit\*.pdb"') do "%RootDirectory%\dump_syms.exe" %%f > "%RootDirectory%\syms\%%~nf.sym"

rem -> move the files who are unique to the client dir
move /y "%RootDirectory%\syms\obs_studio_client.sym" "%RootDirectory%\syms\client\obs_studio_client.sym"
move /y "%RootDirectory%\syms\obs-frontend-api.sym" "%RootDirectory%\syms\client\obs-frontend-api.sym"

rem -> move the files who are unique to the server dir
move /y "%RootDirectory%\syms\obs64.sym" "%RootDirectory%\syms\server\obs64.sym"
move /y "%RootDirectory%\syms\w32-pthreads.sym" "%RootDirectory%\syms\server\w32-pthreads.sym"
move /y "%RootDirectory%\syms\obsglad.sym" "%RootDirectory%\syms\server\obsglad.sym"
move /y "%RootDirectory%\syms\libobs-d3d11.sym" "%RootDirectory%\syms\server\libobs-d3d11.sym"
move /y "%RootDirectory%\syms\libobs-opengl.sym" "%RootDirectory%\syms\server\libobs-opengl.sym"
move /y "%RootDirectory%\syms\coreaudio-encoder.sym" "%RootDirectory%\syms\server\coreaudio-encoder.sym"
move /y "%RootDirectory%\syms\obs.sym" "%RootDirectory%\syms\server\obs.sym"
move /y "%RootDirectory%\syms\obsglad.sym" "%RootDirectory%\syms\server\obsglad.sym"
move /y "%RootDirectory%\syms\enc-amf.sym" "%RootDirectory%\syms\server\enc-amf.sym"
move /y "%RootDirectory%\syms\facemask_AVX.sym" "%RootDirectory%\syms\server\facemask_AVX.sym"
move /y "%RootDirectory%\syms\facemask_NO_AVX.sym" "%RootDirectory%\syms\server\facemask_NO_AVX.sym"
move /y "%RootDirectory%\syms\facemask-plugin.sym" "%RootDirectory%\syms\server\facemask-plugin.sym"
move /y "%RootDirectory%\syms\image-source.sym" "%RootDirectory%\syms\server\image-source.sym"
move /y "%RootDirectory%\syms\obs-browser-page.sym" "%RootDirectory%\syms\server\obs-browser-page.sym"
move /y "%RootDirectory%\syms\obs-browser.sym" "%RootDirectory%\syms\server\obs-browser.sym"
move /y "%RootDirectory%\syms\obs-ffmpeg.sym" "%RootDirectory%\syms\server\obs-ffmpeg.sym"
move /y "%RootDirectory%\syms\obs-filters.sym" "%RootDirectory%\syms\server\obs-filters.sym"
move /y "%RootDirectory%\syms\obs-ndi.sym" "%RootDirectory%\syms\server\obs-ndi.sym"
move /y "%RootDirectory%\syms\obs-outputs.sym" "%RootDirectory%\syms\server\obs-outputs.sym"
move /y "%RootDirectory%\syms\obs-qsv11.sym" "%RootDirectory%\syms\server\obs-qsv11.sym"
move /y "%RootDirectory%\syms\obs-text.sym" "%RootDirectory%\syms\server\obs-text.sym"
move /y "%RootDirectory%\syms\obs-transitions.sym" "%RootDirectory%\syms\server\obs-transitions.sym"
move /y "%RootDirectory%\syms\obs-vst.sym" "%RootDirectory%\syms\server\obs-vst.sym"
move /y "%RootDirectory%\syms\obs-x264.sym" "%RootDirectory%\syms\server\obs-x264.sym"
move /y "%RootDirectory%\syms\vlc-video.sym" "%RootDirectory%\syms\server\vlc-video.sym"
move /y "%RootDirectory%\syms\rtmp-services.sym" "%RootDirectory%\syms\server\rtmp-services.sym"
move /y "%RootDirectory%\syms\text-freetype2.sym" "%RootDirectory%\syms\server\text-freetype2.sym"
move /y "%RootDirectory%\syms\win-capture.sym" "%RootDirectory%\syms\server\win-capture.sym"
move /y "%RootDirectory%\syms\win-decklink.sym" "%RootDirectory%\syms\server\win-decklink.sym"
move /y "%RootDirectory%\syms\win-dshow.sym" "%RootDirectory%\syms\server\win-dshow.sym"
move /y "%RootDirectory%\syms\win-mf.sym" "%RootDirectory%\syms\server\win-mf.sym"
move /y "%RootDirectory%\syms\win-wasapi.sym" "%RootDirectory%\syms\server\win-wasapi.sym"

xcopy /y %RootDirectory%\syms %RootDirectory%\syms\client
xcopy /y %RootDirectory%\syms %RootDirectory%\syms\server

"%RootDirectory%\sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server "syms\server"
"%RootDirectory%\sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server-preview "syms\server"
"%RootDirectory%\sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-client "syms\client"