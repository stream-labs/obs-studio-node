const git: any = require('simple-git')();
const gulp: any = require('gulp');
const unzip: any = require('gulp-unzip');
import * as shell from 'shelljs';
import * as https from 'https';
import * as fs from 'fs';
import * as os from 'os';

console.log(shell.env);

/* This can work on other platforms with some modifications:
    1. Remove dependency download for platforms that don't use it. 
    
   That's it!
 */

const obsGitURL = 'git@github.com:twitchalerts/obs-studio_fork.git';
const obsPath = require('path').join(__dirname, 'obs-build');
const obsDepsZipPath = `${obsPath}/dependencies2015.zip`;
const obsDepsPath = `${obsPath}/dependencies2015`;
const obsDepsPath64 = `${obsDepsPath}/win64`;
const obsBuild64 = `${obsPath}/build64`;

let configType = shell.env['npm_config_cmake_OBS_BUILD_TYPE'] || 'Release';
let obsGenerator = shell.env['npm_config_OSN_GENERATOR'];

function obsBuild() {
    let buildCmd = `cmake --build ${obsBuild64} --config ${configType}`;
    console.log(buildCmd);
    shell.exec(buildCmd);
}

function finishConfigure(error: any, stdout: string, stderr: string) {
    if (error) {
        console.log(`Failed to execute cmake: ${error}`);
        return;
    }

    console.log(`${stdout}`)
    obsBuild();
}

/* Just assume cmake is available in $PATH */
function obsConfigure() {

    let generator: string;

    if (obsGenerator)
        generator = `-G${obsGenerator}`;
    else if (os.platform() == 'win32')
        generator = `-G"Visual Studio 14 2015 Win64"`
    else {
        console.log(`Unsupported platform!`);
        return;
    }

    const configCmd = `cmake ${generator} -DENABLE_UI=false -DDepsPath=${obsDepsPath64} -H${obsPath} -B${obsBuild64}`;

    console.log(configCmd);

    shell.exec(configCmd, { async: true }, finishConfigure);
}

function unpackObsDeps() {
    console.log(`Unpacking ${obsDepsZipPath}`);

    gulp.src(`${obsDepsZipPath}`)
        .pipe(unzip())
        .pipe(gulp.dest(`${obsDepsPath}`));

    obsConfigure();
}

function downloadObsDeps(missing: any) {
    /* Already exists, assume it's okay
     * TODO: Perform checksum. */
    if (!missing) {
        unpackObsDeps();
        return;
    }

    let file =  fs.createWriteStream(obsDepsZipPath);
    let reqUrl = 'https://obsproject.com/downloads/dependencies2015.zip';
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

    req.on('error', (error) => {
        console.log('Failed to download dependencies!');
    });
}

function checkObsDeps() {
    fs.access(obsDepsZipPath, fs.constants.F_OK, downloadObsDeps);
}

function obsUpdateModulesFinish(error: any, data: any) {
    if (error) {
        console.log(`Failed to update submodules.`);
        return;
    }

    checkObsDeps();
}

function obsUpdateModules() {
    git.cwd(obsPath);

    let submodules = [
        'plugins/enc-amf',
        'plugins/mac-syphon/syphon-framework',
        'plugins/win-dshow/libdshowcapture'
    ];

    console.log(`Updating submodules: ${submodules}`)

    git.submoduleUpdate(
        ['--init'].concat(submodules), 
        obsUpdateModulesFinish);
}

function obsAlreadyCloned() {
    obsUpdateModules();
}

function obsCloneFinish(error: any, data: any) {
    if (error) return;

    obsUpdateModules();
}

function obsClone(error: any) {
    if (error) {
        console.log(`Cloning ${obsPath}`);
        git.clone(obsGitURL, obsPath, ['-b', 'slobs-npm-package'], obsCloneFinish);
    }
    else {
        console.log(`${obsPath} already exists.`);
        obsAlreadyCloned();
    }
}

fs.access(obsPath, fs.constants.F_OK, obsClone);