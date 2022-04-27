rem ##################
rem Script to check changes in dependencies
rem It compares dependencies of a module after a build to saved list

rem ##################
rem Prepare file names
Set BinaryFilename=%1
For %%A in ("%BinaryFilename%") do (
  Set BinaryName=%%~nxA
  Set depsFile=%3/dependencies/%%~nxA.txt
  Set depsCurrent=%2/%%~nxA
)

rem ##################
rem Get dependencies from current binary
dumpbin /DEPENDENTS %BinaryFilename% > %depsCurrent%_1.txt

rem ##################
rem Clean up parts of a dependency report what could change
powershell -command "Get-Content %depsCurrent%_1.txt | Select-String -CaseSensitive -Pattern dependencies -Context 0,999 > %depsCurrent%_2.txt"
powershell -command "Get-Content %depsCurrent%_2.txt | Select-String -CaseSensitive -Pattern Summary -Context 999,0 > %depsCurrent%.txt"
powershell -command "Remove-Item %depsCurrent%_1.txt"
powershell -command "Remove-Item %depsCurrent%_2.txt"

IF EXIST %depsFile% (
  rem ##################
  rem Compare current dependency list with saved
  rem Write-Error should stop cmake with an error
  powershell -command "compare-object (get-content %depsCurrent%.txt) (get-content %depsFile%)"
  powershell -command "if( compare-object (get-content %depsCurrent%.txt) (get-content  %depsFile%) ) " ^
   "{ " ^
      "Write-Host '!! !! !! !! !! !!' -ForegroundColor Green;" ^
      "Write-Output '  Dependencies changed for %BinaryName%';" ^
      "Write-Output '  Saved dependencies %depsFile%';" ^
      "Write-Output '  New dependencies %depsCurrent%.txt';" ^
      "Write-Host '!! !! !! !! !! !!' -ForegroundColor Green;" ^
      "Write-Output '';" ^
      "Write-Error 'Alarm: Dependencies changed. Please review changes.' " ^
   "} "
) ELSE (
  rem ##################
  powershell -command ^
      "Write-Host '!! !! !! !! !! !!' -ForegroundColor Green;" ^
      "Write-Output '  Dependencies list was not created for %BinaryName%';" ^
      "Write-Output '  Current dependencies %depsCurrent%.txt';" ^
      "Write-Output '  Review and save as %depsFile%';" ^
      "Write-Host '!! !! !! !! !! !!' -ForegroundColor Green;" ^
      "Write-Output '' "
  powershell -command "Write-Error 'Alarm: Dependencies list was not created.'"
)


