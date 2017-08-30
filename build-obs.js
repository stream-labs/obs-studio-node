"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const git = require('simple-git')();
const url = require('url');
const unzip = require('unzipper');
const path = require("path");
const shell = require("shelljs");
const process = require("process");
const https = require("https");
const fs = require("fs");
const os = require("os");
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
function finishBuild(error, stdout, stderr) {
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
    shell.exec(buildCmd, { async: true, silent: true }, finishBuild);
}
function finishConfigure(error, stdout, stderr) {
    if (error) {
        console.log(`Failed to execute cmake: ${error}`);
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        process.exit(1);
    }
    obsBuild();
}
function obsConfigure() {
    let generator;
    if (obsGenerator)
        generator = `-G"${obsGenerator}"`;
    else if (os.platform() == 'win32')
        generator = `-G"Visual Studio 14 2015 Win64"`;
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
function downloadObsDeps(missing) {
    if (!missing) {
        unpackObsDeps();
        return;
    }
    let file = fs.createWriteStream(obsDepsZipPath);
    let reqUrl = url.parse('https://obsproject.com/downloads/dependencies2015.zip');
    let reqFinish = (response) => {
        console.log(`Saving file to ${obsDepsZipPath}`);
        response.on('data', (data) => {
            file.write(data);
        });
        response.on('end', () => {
            unpackObsDeps();
        });
    };
    let req = https.get(reqUrl, reqFinish);
    req.on('error', (error) => {
        console.log('Failed to download dependencies!');
    });
}
fs.access(obsDepsZipPath, downloadObsDeps);
