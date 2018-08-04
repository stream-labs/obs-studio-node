// Calling obs.IPC.Host() twice should result in an exception.
const {obs, Test} = require("../helpers/bootstrap.js")

let self = new Test();
self.test = function(resolve, reject) {
	try {
		console.log("Hosting server...");
		obs.IPC.host("osn-test");
	} catch (e) {
		console.log("Initial host failed.");
		console.error(e);
		resolve(false);
	}

	try {
		console.log("Hosting server again...");
		obs.IPC.host("osn-test");
	} catch (e) {
		obs.IPC.disconnect();
		resolve(true);
		return;
	}

	console.error("Second host succeeded, but should have failed.");
	try {
		console.log("Disconnecting...");
		obs.IPC.disconnect();
	} catch (e) {
		console.error("Failed to disconnect: ", e);
	}
	resolve(false);
}
self.run();
