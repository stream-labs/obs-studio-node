# Install system dependencies
brew update
brew install node

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
