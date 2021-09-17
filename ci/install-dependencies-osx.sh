# Install system dependencies
brew update
brew doctor
brew install cmake
brew install python
brew install node@14
export PATH="/usr/local/opt/node@14/bin:$PATH"
node -v

# Install module dependencies
yarn install
yarn add electron@${ELECTRONVERSION}