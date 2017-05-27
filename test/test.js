const {app, BrowserWindow} = require('electron');
var path = require('path');
const { obs } = require('../dist/obs-node');

console.log(obs);

app.on('ready', () => {
    obs.startup('en-US');
    
    console.log("Status: " + obs.status);
    console.log("Locale: " + obs.locale);

    var version = obs.version
    var major = (version & 0xFF000000) >> 24;
    var minor = (version & 0x00FF0000) >> 16;
    var patch = (version & 0x0000FFFF);
    
    console.log("Version: " + major + "." + minor + "." + patch);

    obs.module.add_path(
        path.resolve('dist'),
        path.resolve('dist/data')
    );

    obs.module.add_path(
        path.resolve('dist/obs-plugins'), 
        path.resolve('dist/data/obs-plugins/%module%/data')
    );

    obs.module.load_all();
    obs.module.log_loaded();

    var input = new obs.Input('monitor_capture', 'test input');

    console.log(input.name);

    /*
    console.log("Output Types\n");
    console.log(obs.output.types());
    */
    var renderer = new BrowserWindow();

    let test_renderer_html = require('url').format({
        protocol: 'file',
        slashes: true,
        pathname: require('path').join(__dirname, 'test.renderer.html')
    })
    renderer.loadURL(test_renderer_html);
    renderer.webContents.openDevTools();

    input.release();
});

app.on('quit', () => {
    obs.shutdown();
})