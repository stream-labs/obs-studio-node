if "%ReleaseName%"=="debug" (
    cmake --build %SLBuildDirectory% --target install --config Debug
) else (
    cmake --build %SLBuildDirectory% --target install --config RelWithDebInfo
)
