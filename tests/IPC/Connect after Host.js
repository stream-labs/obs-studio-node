// Calling connect() after host() should throw an exception and fail.

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
		console.log("Connecting again...");
		obs.IPC.connect("osn-test");
		console.error("Should have failed, but didn't.");
	} catch (e) {
		resolve(true);
	}
	
	try {
		console.log("Disconnecting...");
		obs.IPC.disconnect();
	} catch (e) {
		console.error("Failed to disconnect, exception: ", e);
		resolve(false);
	}
	
	resolve(false);
}
self.run();
