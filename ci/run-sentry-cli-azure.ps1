Get-ChildItem -Path $env:PDBPATH\obs-studio-node -Filter *.pdb -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_} 
Get-ChildItem -Path $env:PDBPATH\obs-studio-node -Filter *.pdb -Recurse -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms\*.pdb | Rename-Item -NewName { $_.Name -Replace ".pdb",".sym"}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms -Filter *.sym -File | ForEach-Object {Get-Content $_.FullName | Out-File -Encoding Ascii "$env:ROOTDIRECTORY\syms\ascii\$($_.BaseName).sym" } 

Move-Item -Path  $env:ROOTDIRECTORY\syms\ascii\obs_studio_client.sym -Destination $env:ROOTDIRECTORY\syms\ascii\client
Move-Item -Path  $env:ROOTDIRECTORY\syms\ascii\obs-frontend-api.sym -Destination $env:ROOTDIRECTORY\syms\ascii\client

.\sentry-cli.exe upload-dif --org streamlabs-obs --project obs-server "$env:ROOTDIRECTORY\syms\ascii"
.\sentry-cli.exe upload-dif --org streamlabs-obs --project obs-server-preview "$env:ROOTDIRECTORY\syms\ascii"
.\sentry-cli.exe upload-dif --org streamlabs-obs --project obs-client "$env:ROOTDIRECTORY\syms\ascii\client"
