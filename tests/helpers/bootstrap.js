"use strict";
Object.defineProperty(exports, "__esModule", { value: true });

const obs = require("../../streamlabs-build/distribute/obs-studio-node")
const {app} = require('electron')

class CTest {	
	constructor() {
		this.test = function(resolve, reject) {
			console.log("Default test, replace me.");
			resolve(true);
		}
	}
	
	runTest() {
		return new Promise(this.test);
	}
	
	async run() {
		try {
			let res = await this.runTest();
			if (res == false) {
				app.exit(1);
			}
			app.exit(0);
		} catch(e) {
			console.error("Uncaught exception: ", e);
			app.exit(-1);
		}
	}
}

exports.obs = obs;
exports.Test = CTest;