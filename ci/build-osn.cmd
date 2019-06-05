if "%Build.Reason%"=="PullRequest" (
    'cmake.exe --build %SLBuildDirectory% --target install --config Debug'
) else (
    'cmake.exe --build %SLBuildDirectory% --target install --config RelWithDebInfo'
)
