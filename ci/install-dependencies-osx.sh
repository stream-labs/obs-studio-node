# Install system dependencies
brew update
brew doctor
brew upgrade node

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
