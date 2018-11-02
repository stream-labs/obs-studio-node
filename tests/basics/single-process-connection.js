// Calling connect() after host() should throw an exception and fail.

const {uuid, obs, TestGroup} = require("../helpers/bootstrap.js")

let tg = new TestGroup();

tg.addTest("Host & Disconnect", (resolve, reject) => {
	let ipc_name = "obs" + uuid();

	try {
		obs.IPC.host(ipc_name)
	} catch (e) {
		reject("Failed to host server, " + e);
		resolve(false);
		return;
	}

	try {
		obs.IPC.disconnect();
	} catch (e) {
		reject("Failed to disconnect, " + e);
		resolve(false);
		return;
	}

	resolve(true);
});

tg.addTest("Host, Connect & Disconnect", (resolve, reject) => {
	let ipc_name = "obs" + uuid();

	try {
		obs.IPC.host(ipc_name)
	} catch (e) {
		reject("Failed to host server, " + e);
		resolve(false);
		return;
	}
	
	try {
		obs.IPC.connect(ipc_name)
		reject("Connected twice, " + e);
		resolve(false);
		obs.IPC.disconnect();
		return;
	} catch (e) {
	}

	try {
		obs.IPC.disconnect();
	} catch (e) {
		reject("Failed to disconnect, " + e);
		resolve(false);
		return;
	}

	resolve(true);
});

tg.addTest("Host, Host & Disconnect", (resolve, reject) => {
	let ipc_name = "obs" + uuid();
	let ipc_name2 = "obs" + uuid();

	try {
		obs.IPC.host(ipc_name)
	} catch (e) {
		reject("Failed to host server, " + e);
		resolve(false);
		return;
	}
	
	try {
		obs.IPC.host(ipc_name2)
		reject("Hosted twice, " + e);
		resolve(false);
		obs.IPC.disconnect();
		return;
	} catch (e) {
	}

	try {
		obs.IPC.disconnect();
	} catch (e) {
		reject("Failed to disconnect, " + e);
		resolve(false);
		return;
	}

	resolve(true);
});

tg.run();