import os
os.system('curl -sL https://sentry.io/get-cli/ | bash')

def process_sentry(project, directory):
    for r, d, f in os.walk(directory):
        for file in f:
            if 'lib' in file or 'obs' in file or '.so' in file or '.dylib' in file:
                path = os.path.join(directory, file)
                os.system("dsymutil " + path)
                os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project " + project + " " + path + ".dSYM/Contents/Resources/DWARF/" + file)
    
# # Upload client debug files
# client_path = "$PWD/${SLBUILDDIRECTORY}/obs-studio-client/${BUILDCONFIG}/"
# client_name = "obs_studio_client.0.3.21.0.node"

# os.system("dsymutil " + client_path + client_name)
# os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-client " +
# client_path + client_name + ".dSYM/Contents/Resources/DWARF/" + client_name)

# # Upload client debug files
process_sentry('obs-client', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-client', os.environ['BUILDCONFIG']))
# # Upload server debug files
process_sentry('obs-server', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-server', os.environ['BUILDCONFIG']))


# # Upload obs debug files
# obs_bin_path = os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'libobs-src', 'bin')
# for r, d, f in os.walk(obs_bin_path):
#     for file in f:
#         if 'lib' in file or 'obs' in file:
#             path = os.path.join(obs_bin_path, file)
#             os.system('dsymutil ' + path)
#             os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server " + path + ".dSYM/Contents/Resources/DWARF/" + file)

# # # Upload obs-plugins debug files
# obs_plugins_path = os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'libobs-src', 'obs-plugins')
# for r, d, f in os.walk(obs_plugins_path):
#     for file in f:
#         if '.so' in file:
#             path = os.path.join(obs_plugins_path, file)
#             os.system("dsymutil " + path)
#             os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-obs --project obs-server " + path + ".dSYM/Contents/Resources/DWARF/" + file)