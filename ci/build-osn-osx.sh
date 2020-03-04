cmake --build ${SLBUILDDIRECTORY} --target install --config ${BUILDCONFIG}

curl -sL https://sentry.io/get-cli/ | bash
dsymutil $PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/obs_studio_client.0.3.21.0.node
sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-client $PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/obs_studio_client.0.3.21.0.node.dSYM/Contents/Resources/DWARF/obs_studio_client.0.3.21.0.node

dsymutil $PWD/${SLBUILDDIRECTORY}/obs-studio-server/${BUILDCONFIG}/obs64
sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server $PWD/${SLBUILDDIRECTORY}/obs-studio-server/${BUILDCONFIG}/obs64.dSYM/Contents/Resources/DWARF/obs64
