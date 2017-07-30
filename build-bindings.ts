import * as shell from 'shelljs';
import * as path from 'path';
import * as os from 'os';

let configType = shell.env['npm_config_cmake_OBS_BUILD_TYPE'] || 'Release';
let obsGenerator = shell.env['npm_config_OSN_GENERATOR'];

let npm_bin_path: string;

function finishInstall(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        return;
    }
}

function installBindings() {
    let installCmd = `cmake --build build --config ${configType} --target install`;

    console.log(installCmd);

    shell.exec(installCmd, { silent: true, async: true }, finishInstall);
}

function finishBuild(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        return;
    }

    installBindings();
}

function buildBindings() {
    let buildCmd = `cmake --build build --config ${configType}`;

    console.log(buildCmd);

    shell.exec(buildCmd, { silent: true, async: true }, finishBuild);
}

function finishConfigure(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to exec cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        return;
    }

    console.log(stdout);

    buildBindings();
}

function configureBindings() {
    let generator: string;

    if (obsGenerator)
        generator = `-G${obsGenerator}`;
    else if (os.platform() == 'win32')
        generator = `-G"Visual Studio 14 2015 Win64"`
    else {
        console.log(`Unsupported platform!`);
        return;
    }
    let cmake_js_path = path.normalize(`${npm_bin_path}/cmake-js`);
    let configureCmd = `${cmake_js_path} configure ${generator}`;

    console.log(configureCmd);

    shell.exec(configureCmd, { async: true, silent: true}, finishConfigure);
}

shell.exec('npm bin', { async: true, silent:true}, (error, stdout, stderr) => {
    if (error) {
        console.log(`Failed to fetch npm bin path: ${error}`);
        return;
    }

    npm_bin_path = stdout.trim();
    configureBindings();
});