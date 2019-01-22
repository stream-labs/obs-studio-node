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
move "obs_studio_client.sym" "client\obs_studio_client.sym"
move "obs-frontend-api.sym" "client\obs-frontend-api.sym"

rem -> move the files who are unique to the server dir
copy "obs64.sym" "server\obs64.sym"
copy "libobs-d3d11.sym" "server\libobs-d3d11.sym"
copy "libobs-opengl.sym" "server\libobs-opengl.sym"
copy "obs.sym" "server\obs.sym"
copy "obsglad.sym" "server\obsglad.sym"
copy "enc-amf.sym" "server\enc-amf.sym"
copy "facemask-plugin.sym" "server\facemask-plugin.sym"
copy "image-source.sym" "server\image-source.sym"
copy "obs-browser-page.sym" "server\obs-browser-page.sym"
copy "obs-browser.sym" "server\obs-browser.sym"
copy "obs-ffmpeg.sym" "server\obs-ffmpeg.sym"
copy "obs-filters.sym" "server\obs-filters.sym"
copy "obs-ndi.sym" "server\obs-ndi.sym"
copy "obs-outputs.sym" "server\obs-outputs.sym"
copy "obs-qsv11.sym" "server\obs-qsv11.sym"
copy "obs-text.sym" "server\obs-text.sym"
copy "obs-transitions.sym" "server\obs-transitions.sym"
copy "obs-vst.sym" "server\obs-vst.sym"
copy "obs-x264.sym" "server\obs-x264.sym"
copy "rtmp-services.sym" "server\rtmp-services.sym"
copy "text-freetype2.sym" "server\text-freetype2.sym"
copy "win-capture.sym" "server\win-capture.sym"
copy "win-decklink.sym" "server\win-decklink.sym"
copy "win-dshow.sym" "server\win-dshow.sym"
copy "win-mf.sym" "server\win-mf.sym"
copy "win-wasapi.sym" "server\win-wasapi.sym"

cd ..
xcopy syms syms\client
xcopy syms syms\server

"sentry-cli.exe" --auth-token "e035c0e6c9b74444a1765da04cf85c35f019520a897249ffa1ba17e9ffb3cd5b" upload-dif --org streamlabs-obs --project obs-server "syms\server"
"sentry-cli.exe" --auth-token "e035c0e6c9b74444a1765da04cf85c35f019520a897249ffa1ba17e9ffb3cd5b" upload-dif --org streamlabs-obs --project obs-client "syms\client"