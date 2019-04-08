md syms
md syms\client
md syms\server

rem -> copy all .pdb files from the root dir
cd "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\"
for /f %%f in ('dir /b "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\*.pdb"') do "\projects\obs-studio-node\dump_syms.exe" %%f > "\projects\obs-studio-node\syms\%%~nf.sym"

rem -> copy all .pdb files from plugins dir
cd "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\obs-plugins\64bit\"
for /f %%f in ('dir /b "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\obs-plugins\64bit\*.pdb"') do "\projects\obs-studio-node\dump_syms.exe" %%f > "\projects\obs-studio-node\syms\%%~nf.sym"

cd "\projects\obs-studio-node\syms"

rem -> move the files who are unique to the client dir
move /y "obs_studio_client.sym" "client\obs_studio_client.sym"
move /y "obs-frontend-api.sym" "client\obs-frontend-api.sym"

rem -> move the files who are unique to the server dir
move /y "obs64.sym" "server\obs64.sym"
move /y "w32-pthreads.sym" "server\w32-pthreads.sym"
move /y "obsglad.sym" "server\obsglad.sym"
move /y "libobs-d3d11.sym" "server\libobs-d3d11.sym"
move /y "libobs-opengl.sym" "server\libobs-opengl.sym"
move /y "coreaudio-encoder.sym" "server\coreaudio-encoder.sym"
move /y "obs.sym" "server\obs.sym"
move /y "obsglad.sym" "server\obsglad.sym"
move /y "enc-amf.sym" "server\enc-amf.sym"
move /y "facemask_AVX.sym" "server\facemask_AVX.sym"
move /y "facemask_NO_AVX.sym" "server\facemask_NO_AVX.sym"
move /y "facemask-plugin.sym" "server\facemask-plugin.sym"
move /y "image-source.sym" "server\image-source.sym"
move /y "obs-browser-page.sym" "server\obs-browser-page.sym"
move /y "obs-browser.sym" "server\obs-browser.sym"
move /y "obs-ffmpeg.sym" "server\obs-ffmpeg.sym"
move /y "obs-filters.sym" "server\obs-filters.sym"
move /y "obs-ndi.sym" "server\obs-ndi.sym"
move /y "obs-outputs.sym" "server\obs-outputs.sym"
move /y "obs-qsv11.sym" "server\obs-qsv11.sym"
move /y "obs-text.sym" "server\obs-text.sym"
move /y "obs-transitions.sym" "server\obs-transitions.sym"
move /y "obs-vst.sym" "server\obs-vst.sym"
move /y "obs-x264.sym" "server\obs-x264.sym"
move /y "vlc-video.sym" "server\vlc-video.sym"
move /y "rtmp-services.sym" "server\rtmp-services.sym"
move /y "text-freetype2.sym" "server\text-freetype2.sym"
move /y "win-capture.sym" "server\win-capture.sym"
move /y "win-decklink.sym" "server\win-decklink.sym"
move /y "win-dshow.sym" "server\win-dshow.sym"
move /y "win-mf.sym" "server\win-mf.sym"
move /y "win-wasapi.sym" "server\win-wasapi.sym"

cd ..
xcopy /y syms syms\client
xcopy /y syms syms\server

"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server "syms\server"
"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server-preview "syms\server"
"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-client "syms\client"