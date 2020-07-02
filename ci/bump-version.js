const sh = require('shelljs');
const colors = require('colors/safe');
const fs = require('fs');
const path = require('path');

function log_info(msg) {
    sh.echo(colors.magenta(msg));
  }
  
  function log_error(msg) {
    sh.echo(colors.red(`ERROR: ${msg}`));
  }

function executeCmd(cmd, options = {}) {
    // Default is to exit on failure
    if (options.exit == null) options.exit = true;
  
    const result = options.silent ? sh.exec(cmd, { silent: true }) : sh.exec(cmd);
  
    if (result.code !== 0) {
      log_error(`Command Failed >>> ${cmd}`);
      if (options.exit) {
        sh.exit(1);
      } else {
        throw new Error(`Failed to execute command: ${cmd}`);
      }
    }
  
    return result.stdout;
  }

const newVersion = executeCmd('git describe --tags --abbrev=0', { silent: true })
    .trim()
    .replace(/^v/, '');

try {
    const file = path.join(process.cwd(), 'package.json');
    const jsonData = fs.readFileSync(file);
    const root = JSON.parse(jsonData.toString());
    const currentVersion = root['version'];

    log_info('Current version: ' + currentVersion);
    log_info('New version: ' + newVersion);

    root['version'] = newVersion;
    log_info('Bumping version number...');
    fs.writeFileSync(file, JSON.stringify(root, null, 2));
} catch (error) {
    log_error(error);
    sh.exit(1);
}