# Install system dependencies
brew update
brew install node@12
echo 'export PATH="/usr/local/opt/node@12/bin:$PATH"' >> ~/.bash_profile

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
