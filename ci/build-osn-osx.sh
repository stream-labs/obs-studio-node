cmake --build ${SLBUILDDIRECTORY} --target install --config ${BUILDCONFIG}

curl -sL https://sentry.io/get-cli/ | bash
dsymutil $PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/obs_studio_client.node
sentry-cli --auth-token "$(SENTRYAUTH)" upload-dif --org streamlabs-obs --project obs-client $PWD/${SLBUILDDIRECTORY}/obs_studio_client.node.dSYM/Contents/Resources/DWARF/obs_studio_client.node