const git: any = require('simple-git')();
const url = require('url');
const unzip: any = require('unzipper');
import * as path from 'path';
import * as shell from 'shelljs';
import * as process from 'process';
import * as https from 'https';
import * as fs from 'fs';
import * as os from 'os';

/* This can work on other platforms with some modifications:
    1. Remove dependency download for platforms that don't use it. 
    
   That's it!
 */

const obsPath = path.join(__dirname, 'obs-studio');
const obsDepsZipPath = path.join(__dirname, `dependencies2015.zip`);
const obsDepsPath = path.join(__dirname, `dependencies2015`);
const obsDepsPath64 = path.join(`${obsDepsPath}`, `win64`);
const obsBuild64 = path.join(__dirname, `obs-build64`);

let configType = shell.env['npm_config_cmake_OBS_BUILD_TYPE'] || 'Release';
let obsGenerator = shell.env['npm_config_OSN_GENERATOR'];

if (configType === 'Release') {
    configType = 'RelWithDebInfo';
}

function finishBuild(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to execute cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        process.exit(1);
    }
}

function obsBuild() {
    let buildCmd = `cmake --build \"${obsBuild64}\" --config ${configType}`;
    console.log(buildCmd);
    shell.exec(buildCmd, { async: true, silent: true}, finishBuild);
}

function finishConfigure(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to execute cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        process.exit(1);
    }

    obsBuild();
}

/* Just assume cmake is available in $PATH */
function obsConfigure() {

    let generator: string;

    if (obsGenerator)
        generator = `-G"${obsGenerator}"`;
    else if (os.platform() == 'win32')
        generator = `-G"Visual Studio 14 2015 Win64"`
    else {
        console.log(`Unsupported platform!`);
        process.exit(1);
    }

    const configCmd = `cmake ${generator} -DENABLE_UI=false -DDepsPath="${obsDepsPath64}" -H"${obsPath}" -B"${obsBuild64}"`;

    console.log(configCmd);

    shell.exec(configCmd, { async: true, silent: true }, finishConfigure);
}

function unpackObsDeps() {
    console.log(`Unpacking ${obsDepsZipPath}`);

    fs.createReadStream(`${obsDepsZipPath}`)
        .pipe(unzip.Extract({ path: `${obsDepsPath}` })
            .once('close', () => {
                obsConfigure();
            }));
}

function downloadObsDeps(missing: any) {
    /* Already exists, assume it's okay
     * TODO: Perform checksum. */
    if (!missing) {
        unpackObsDeps();
        return;
    }

    let file =  fs.createWriteStream(obsDepsZipPath);
    let reqUrl = url.parse('https://obsproject.com/downloads/dependencies2015.zip');

    let reqFinish = (response: any) => {
        console.log(`Saving file to ${obsDepsZipPath}`);
        response.on('data', (data: any) => {
            file.write(data);
        });

        response.on('end', () => {
            unpackObsDeps();
        });
    };

    let req = https.get(reqUrl, reqFinish);

    req.on('error', (error: any) => {
        console.log('Failed to download dependencies!');
    });
}

fs.access(obsDepsZipPath, downloadObsDeps);