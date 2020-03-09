import os
os.system('curl -sL https://sentry.io/get-cli/ | bash')

# Upload client debug files
client_path = "$PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/"
client_name = "obs_studio_client.0.3.21.0.node"

os.system("dsymutil " + client_path + client_name)
os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-client " +
client_path + client_name + ".dSYM/Contents/Resources/DWARF/" + client_name)
