import os
os.system('curl -sL https://sentry.io/get-cli/ | bash')

def process_sentry(project, directory):
    print("precess sentry for path {}".format(directory))
    for r, d, f in os.walk(directory):
        for file in f:
            if 'lib' in file or 'obs' in file or '.so' in file or '.dylib' in file:
                path = os.path.join(directory, file)
                print("processing file {}".format(path))
                os.system("dsymutil " + path)
                os.system("sentry-cli --auth-token ${SENTRY_AUTH_TOKEN} upload-dif --org streamlabs-desktop --project " + project + " " + path + ".dSYM/Contents/Resources/DWARF/" + file)

# # Upload client debug files
process_sentry('obs-client', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-client', os.environ['BUILDCONFIG']))
# # Upload server debug files
process_sentry('obs-server', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-server', os.environ['BUILDCONFIG']))

# # Upload client debug files
process_sentry('obs-client-preview', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-client', os.environ['BUILDCONFIG']))
# # Upload server debug files
process_sentry('obs-server-preview', os.path.join(os.environ['PWD'], os.environ['SLBUILDDIRECTORY'], 'obs-studio-server', os.environ['BUILDCONFIG']))