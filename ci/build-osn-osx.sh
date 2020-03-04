cmake --build ${SLBUILDDIRECTORY} --target install --config ${BUILDCONFIG}

dsymutil build/obs-studio-client/${BUILDCONFIG}/obs_studio_client.node
sentry-cli --auth-token "$(SENTRYAUTH)" upload-dif --org streamlabs-obs --project obs-client build/obs-studio-client/${BUILDCONFIG}/obs_studio_client.node.dSYM/Contents/Resources/DWARF/obs_studio_client.node