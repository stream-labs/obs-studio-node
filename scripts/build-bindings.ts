const shell = require('shelljs');
import * as process from 'process';
import * as path from 'path';
import * as os from 'os';

let configType = shell.env['npm_config_OBS_BUILD_TYPE'] || 'Release';
let obsGenerator = shell.env['npm_config_OSN_GENERATOR'];

let npm_bin_path: string;

function finishInstall(error: any, stdout: string, stderr: string) {
    shell.ShellString(stdout).to(`logs/bindings.install.stdout.txt`);
    shell.ShellString(stdout).to(`logs/bindings.install.stderr.txt`);

    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        process.exit(1);
    }
}

function installBindings() {
    let installCmd = `cmake --build "${path.resolve(__dirname, 'build')}" --config ${configType} --target install`;

    console.log(installCmd);

    shell.exec(installCmd, { silent: true, async: true }, finishInstall);
}

function finishBuild(error: any, stdout: string, stderr: string) {
    shell.ShellString(stdout).to(`logs/bindings.build.stdout.txt`);
    shell.ShellString(stdout).to(`logs/bindings.build.stderr.txt`);

    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        process.exit(1);
    }

    installBindings();
}

function buildBindings() {
    let buildCmd = `cmake --build "${path.resolve(__dirname, 'build')}" --config ${configType}`;

    console.log(buildCmd);

    shell.exec(buildCmd, { silent: true, async: true }, finishBuild);
}

function finishConfigure(error: any, stdout: string, stderr: string) {
    shell.ShellString(stdout).to(`logs/bindings.configure.stdout.txt`);
    shell.ShellString(stdout).to(`logs/bindings.configure.stderr.txt`);

    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        process.exit(1);
    }

    console.log(stdout);

    buildBindings();
}

function configureBindings() {
    let generator: string;

    if (obsGenerator)
        generator = `-G"${obsGenerator}"`;
    else if (os.platform() == 'win32')
        generator = `-G"Visual Studio 14 2015 Win64"`
    else {
        console.log(`Unsupported platform!`);
        process.exit(1);
    }
    let cmake_js_path = path.normalize(`${npm_bin_path}/cmake-js`);
    let configureCmd = `"${cmake_js_path}" configure ${generator} -d "${__dirname}"`;

    console.log(configureCmd);

    shell.exec(configureCmd, { async: true, silent: true}, finishConfigure);
}

shell.mkdir(`logs`);

shell.exec('npm bin', { async: true, silent:true}, (error: any, stdout: string, stderr: string) => {
    if (error) {
        console.log(`Failed to fetch npm bin path: ${error}`);
        process.exit(1);
    }

    npm_bin_path = stdout.trim();
    configureBindings();
});