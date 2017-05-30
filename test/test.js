const {app, BrowserWindow} = require('electron');
var path = require('path');
const { obs } = require('../node-obs/obs_node');

let renderer;
let test_input;
let test_scene;

function window_ready()
{
    renderer.show();
}

app.on('ready', () => {
    renderer = new BrowserWindow({'show':false});
    /* Wait until window is created to make display. */
    renderer.once('ready-to-show', window_ready);

    let test_renderer_html = require('url').format({
        protocol: 'file',
        slashes: true,
        pathname: require('path').join(__dirname, 'test.renderer.html')
    });
    
    renderer.loadURL(test_renderer_html);
    renderer.webContents.openDevTools();
    /*
    console.log("Output Types\n");
    console.log(obs.output.types());
    */
});

app.on('quit', () => {
    obs.shutdown();
})