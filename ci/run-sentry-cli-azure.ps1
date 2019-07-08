Get-ChildItem -Path $PDBPath\obs-studio-node -Filter *.pdb -File | ForEach-Object {.\dump_syms.exe $_.FullName > $RootDirectory\syms\$_} 
Get-ChildItem -Path $PDBPath\obs-studio-node -Filter *.pdb -Recurse -File | ForEach-Object {.\dump_syms.exe $_.FullName > $RootDirectory\syms\$_}
Get-ChildItem -Path $RootDirectory\syms\*.pdb | Rename-Item -NewName { $_.Name -Replace ".pdb",".sym"}
Get-ChildItem -Path $RootDirectory\syms -Filter *.sym -File | ForEach-Object {Get-Content $_.FullName | Out-File -Encoding Ascii "$RootDirectory\syms\ascii\$($_.BaseName).sym" } 

Move-Item -Path  $RootDirectory\syms\ascii\obs_studio_client.sym -Destination $RootDirectory\syms\ascii\client
Move-Item -Path  $RootDirectory\syms\ascii\obs-frontend-api.sym -Destination $RootDirectory\syms\ascii\client

.\sentry-cli.exe --auth-token "fa314262dfd546d38828a41f7f30b532964a14acf1b541cbad21f95659859c3d" upload-dif --org streamlabs-obs --project obs-server "$RootDirectory\syms\ascii"
.\sentry-cli.exe --auth-token "fa314262dfd546d38828a41f7f30b532964a14acf1b541cbad21f95659859c3d" upload-dif --org streamlabs-obs --project obs-server-preview "$RootDirectory\syms\ascii"
.\sentry-cli.exe --auth-token "fa314262dfd546d38828a41f7f30b532964a14acf1b541cbad21f95659859c3d" upload-dif --org streamlabs-obs --project obs-client "$RootDirectory\syms\ascii\client"
