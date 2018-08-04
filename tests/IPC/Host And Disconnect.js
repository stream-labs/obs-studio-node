// Simple test to see if we can start and stop the IPC build correctly.
// We must make sure that after disconnecting as a host that the server was closed too.
const {obs, Test} = require("../helpers/bootstrap.js")

let self = new Test();
self.test = function(resolve, reject) {
	try {
		console.log("Hosting server...");
		obs.IPC.host("osn-test");
	} catch (e) {
		console.error("Failed to host, exception: ", e);
		resolve(false);
	}
	
	try {
		console.log("Disconnecting...");
		obs.IPC.disconnect();
	} catch (e) {
		console.error("Failed to disconnect, exception: ", e);
		resolve(false);
	}
	
	// The server takes a short while to stop.
	setTimeout(function() {
		try {
			obs.IPC.connect("osn-test");
		} catch (e) {
			resolve(true);
		}
		
		console.error("Successfully reconnected to server, which should have been closed.")
		obs.IPC.disconnect();
		resolve(false);
	}, 1000);
}
self.run();
