# Install system dependencies
brew update
brew doctor
brew install node
brew install nvm
mkdir ~/.nvm
export NVM_DIR=~/.nvm
source $(brew --prefix nvm)/nvm.sh

nvm ls
node -v

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}
