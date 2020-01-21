# Install system dependencies
brew update
brew doctor
brew install node
brew install nvm
mkdir ~/.nvm
export NVM_DIR=~/.nvm
source $(brew --prefix nvm)/nvm.sh
npm config delete prefix
nvm use 12.14.1

# Install module dependencies
yarn install
yarn add electron@${ElectronVersion}


echo "Printing electron version"
echo ${ElectronVersion}