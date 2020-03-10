import os
os.system('curl -sL https://sentry.io/get-cli/ | bash')

# Upload client debug files
client_path = "$PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/"
client_name = "obs_studio_client.0.3.21.0.node"

os.system("dsymutil " + client_path + client_name)
os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-client " +
client_path + client_name + ".dSYM/Contents/Resources/DWARF/" + client_name)

# Upload server debug files
server_path = "$PWD/${SLBUILDDIRECTORY}/obs-studio-server/${BUILDCONFIG}/"
for r, d, f in os.walk(server_path):
    for file in f:
        if '.so' in file or not '.' in file:
            print("Processing " + file + '...')
            os.system("dsymutil " + server_path + file)
            os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server " + server_path + file + ".dSYM/Contents/Resources/DWARF/" + file)

# Upload obs debug files
obs_bin_path = "$PWD/${SLBUILDDIRECTORY}/libobs-src/bin/"
for r, d, f in os.walk(obs_bin_path):
    for file in f:
        if '.dylib' in file or not '.' in file:
            print("Processing " + file + '...')
            os.system("dsymutil " + obs_bin_path + file)
            os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server " + obs_bin_path + file + ".dSYM/Contents/Resources/DWARF/" + file)

# Upload obs-plugins debug files
obs_plugins_path = "$PWD/${SLBUILDDIRECTORY}/libobs-src/obs-plugins/"
for r, d, f in os.walk(obs_plugins_path):
    for file in f:
        if '.so' in file or not '.' in file:
            print("Processing " + file + '...')
            os.system("dsymutil " + obs_plugins_path + file)
            os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server " + obs_plugins_path + file + ".dSYM/Contents/Resources/DWARF/" + file)