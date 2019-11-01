curl -kL "https://s3.amazonaws.com/getsentry-builds/getsentry/breakpad-tools/windows/breakpad-tools-windows.zip" -o "breakpadtools.zip" -f --retry 5
7z x "breakpadtools.zip" > nul

curl -kL "https://github.com/getsentry/sentry-cli/releases/download/1.48.0/sentry-cli-Windows-i686.exe" -o "sentry-cli.exe" -f --retry 5