md syms
cd "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\"
for /f %%f in ('dir /b "\projects\obs-studio-node\streamlabs-build\distribute\obs-studio-node\*.pdb"') do "\projects\obs-studio-node\dump_syms.exe" %%f > "\projects\obs-studio-node\syms\%%~nf.sym"

cd "\projects\obs-studio-node\syms"

"..\sentry-cli.exe" --auth-token "e035c0e6c9b74444a1765da04cf85c35f019520a897249ffa1ba17e9ffb3cd5b" upload-dif --org streamlabs-obs --project obs-server "..\syms"

cd ..