# Install system dependencies
brew update
brew install node@12

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
