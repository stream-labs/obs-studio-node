& secure-file\tools\secure-file -decrypt CI\streamlabsp12.pfx.enc -secret "$env:StreamlabsPfxSecret" -out CI\streamlabsp12.pfx

if ($LASTEXITCODE -ne 0) {
	exit 1
}

Get-ChildItem -Recurse  "$env:SLFullDistributePath" -Include "*.dll","*.node","*.exe" |
Foreach-Object {
	& "$env:SignTool" sign /as /p "$env:StreamlabsSecret" /f CI\streamlabsp12.pfx $_.FullName
	if ($LASTEXITCODE -ne 0) {
		exit 1
	}
}