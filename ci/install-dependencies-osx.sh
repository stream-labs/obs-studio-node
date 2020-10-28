# Install system dependencies
brew update
brew doctor
brew install cmake
brew install python
brew install node
brew install nvm
mkdir ~/.nvm
export NVM_DIR=~/.nvm
source $(brew --prefix nvm)/nvm.sh
npm config delete prefix
nvm install 14.15.0
nvm use 14.15.0

# Install module dependencies
yarn install
yarn add electron@${ELECTRONVERSION}