# Install system dependencies
brew update
brew doctor
brew install cmake
brew install python
node -v

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}