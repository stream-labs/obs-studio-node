# Install system dependencies
brew update
brew doctor
brew install node
node -v

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
