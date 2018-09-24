// Tests with multiple processes (this and a client one).

const process = require("process");
const { fork } = require("child_process");
const { uuid, obs, TestGroup } = require("../helpers/bootstrap.js");
const app = undefined;
try {
    let el = require('electron');
    app = el.app;
} catch (e) {
}

process.stdin.setEncoding('utf8');

function writeLine(text, sock = process.stderr) {
    sock.cork();
    sock.write(text + "\n\0");
    sock.uncork();
}
function writeNull(text, sock = process.stderr) {
    sock.cork();
    sock.write(text + "\0");
    sock.uncork();
}

function handleCommandReply(chunk) {
    let cmds = chunk.split('\0');
    for (let cmd of cmds) {
        switch (cmd) {
            case "ready":
                writeNull(this.startCommand, this.cproc.stdin);
                break;
            case "connect":
            case "connectOrHost":
                writeNull("disconnect", this.cproc.stdin);
                break;
            case "disconnect":
                writeNull("quit", this.cproc.stdin);
                break;
            case "quit":
                try {
                    obs.IPC.disconnect();
                    this.resolve(true);
                } catch (e) {
                    writeLine("Failed to disconnect with exception: " + e);
                    this.reject("Failed to disconnect, " + e);
                    this.resolve(false);
                    return;
                }
                break;
            case "":
                // no-op because all commands end with a \0 which causes an extra entry.
                break;
            default:
                console.log("client: " + cmd);
        }
    }
}

if (process.argv[2] == undefined) {
    let ipc_name = "obs" + uuid();
    let tg = new TestGroup();

    tg.addTest("connect then disconnect", (resolve, reject) => {
        let cproc = undefined;
        let intv = undefined;
        let tmo = undefined;

        try {
            obs.IPC.host(ipc_name)
        } catch (e) {
            reject("Failed to host server, " + e);
            resolve(false);
            return;
        }

        function handleClose(code, signal) {
            if (code != 0) {
                reject("Client process died unexpectedly with code: " + code + ".");
            }
            clearTimeout(tmo);
            clearInterval(intv);
            cproc = undefined;
        }
        function handleStdErr(chunk) {
            console.log("client: " + chunk);
        }
        cproc = fork(__filename, [ipc_name], { silent: true });
        cproc.stdout.cproc = cproc;
        cproc.stdout.resolve = resolve;
        cproc.stdout.reject = reject;
        cproc.stdout.startCommand = "connect";
        cproc.stdin.setEncoding('utf8');
        cproc.stdout.setEncoding('utf8');
        cproc.stderr.setEncoding('utf8');
        cproc.stdout.on('data', handleCommandReply);
        cproc.stderr.on('data', handleStdErr);
        cproc.on('close', handleClose);

        intv = setInterval(function () {
            console.log("Waiting on client...");
        }, 500);
    });

    tg.addTest("connectOrHost then disconnect", (resolve, reject) => {
        let cproc = undefined;
        let intv = undefined;
        let tmo = undefined;

        try {
            obs.IPC.host(ipc_name)
        } catch (e) {
            reject("Failed to host server, " + e);
            resolve(false);
            return;
        }

        function handleClose(code, signal) {
            if (code != 0) {
                reject("Client process died unexpectedly with code: " + code + ".");
            }
            clearTimeout(tmo);
            clearInterval(intv);
            cproc = undefined;
        }
        function handleStdErr(chunk) {
            console.log("client: " + chunk);
        }
        cproc = fork(__filename, [ipc_name], { silent: true });
        cproc.stdout.cproc = cproc;
        cproc.stdout.resolve = resolve;
        cproc.stdout.reject = reject;
        cproc.stdout.startCommand = "connectOrHost";
        cproc.stdin.setEncoding('utf8');
        cproc.stdout.setEncoding('utf8');
        cproc.stderr.setEncoding('utf8');
        cproc.stdout.on('data', handleCommandReply);
        cproc.stderr.on('data', handleStdErr);
        cproc.on('close', handleClose);

        intv = setInterval(function () {
            console.log("Waiting on client...");
        }, 500);
    });

    tg.run();
} else {
    let ipc_name = process.argv[2];
    let intv = undefined;

    process.stdout.setEncoding('utf8');
    process.stderr.setEncoding('utf8');

    process.stdout.write("ready\0");
    function handleStdIn(chunk) {
        let cmds = chunk.split('\0');
        for (let cmd of cmds) {
            try {
                switch (cmd) {
                    case "connect":
                        writeLine("Connecting...");
                        obs.IPC.connect(ipc_name);
                        process.stdout.write("connect\0");
                        break;
                    case "connectOrHost":
                        writeLine("Connecting or hosting...");
                        obs.IPC.connectOrHost(ipc_name);
                        process.stdout.write("connectOrHost\0");
                        break;
                    case "disconnect":
                        writeLine("Disconnecting...");
                        obs.IPC.disconnect();
                        process.stdout.write("disconnect\0");
                        break;
                    case "quit":
                        writeLine("Quitting...");
                        clearInterval(intv);
                        process.stdout.write("quit\0");
                        process.exit(0);
                        break;
                }
            } catch (e) {
                clearInterval(intv);
                writeLine("Exception: " + e);
                if (app) {
                    app.quit(-1);
                } else {
                    process.exit(-1);
                }
                return;
            }
        }
    }
    process.stdin.on('data', handleStdIn);

    intv = setInterval(function () {
    }, 100);
}
