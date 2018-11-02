"use strict";
Object.defineProperty(exports, "__esModule", { value: true });

const {obs} = require('./obs.js')
const process = require('process')
const app = undefined;
try {
	const el = require('electron')
	app = el.app;
} catch (e) {
}

class CTest {	
	constructor(func) {
		if (func === undefined) {
			this.test = function(resolve, reject) {
				console.log("Default test, replace me.");
				resolve(true);
			}
		} else {
			this.test = func;
		}
	}
	
	runTest() {
		return new Promise(this.test);
	}
	
	async run() {
		try {
			let res = await this.runTest();
			if (res == false) {
				if (app) {
					app.quit(1);
				} else {
					process.exit(1);
				}
			}
			if (app) {
				app.quit(0);
			} else {
				process.exit(0);
			}
		} catch(e) {
			console.error(">> Uncaught exception: ", e);
			if (app) {
				app.quit(-1);
			} else {
				process.exit(-1);
			}
		}
	}
}

class CTestGroup {
	constructor(fnInitializer, fnFinalizer) {
		this.tests = new Object();
		this.fnInitializer = fnInitializer;
		this.fnFinalizer = fnFinalizer;
	}
	
	// name: string - Name of the test
	// test: bool function(resolve: function, reject: function) - Function to call asynchronously
	addTest(name, test) {
		this.tests[name] = test;
	}
	
	removeTest(name) {
		this.tests[name] = undefined;
	}
	
	async run() {
		var returnValue = 0;
		if (this.fnInitializer) {
			this.fnInitializer();
		}
		for (var name in this.tests) {
			console.log(">> Running test '" + name + "'...");
			if ((this.tests[name] !== undefined) && (typeof(this.tests[name]) === "function")) {
				try {
					var result = await new Promise(this.tests[name]);
					if (result === false) {
						returnValue = 1;
						break;
					}
				} catch(e) {
					returnValue = -1;
					console.error(">> Uncaught exception: ", e);
					break;
				}
			}
		}
		if (this.fnFinalizer) {
			this.fnFinalizer();
		}
		if (app) {
			app.quit(returnValue);
		} else {
			process.exit(returnValue);
		}
	}
}

var lut = []; for (var i=0; i<256; i++) { lut[i] = (i<16?'0':'')+(i).toString(16); }
function uuid() {
	var d0 = Math.random()*0xffffffff|0;
	var d1 = Math.random()*0xffffffff|0;
	var d2 = Math.random()*0xffffffff|0;
	var d3 = Math.random()*0xffffffff|0;
	return lut[d0&0xff]+lut[d0>>8&0xff]+lut[d0>>16&0xff]+lut[d0>>24&0xff]+'-'+
		lut[d1&0xff]+lut[d1>>8&0xff]+'-'+lut[d1>>16&0x0f|0x40]+lut[d1>>24&0xff]+'-'+
		lut[d2&0x3f|0x80]+lut[d2>>8&0xff]+'-'+lut[d2>>16&0xff]+lut[d2>>24&0xff]+
		lut[d3&0xff]+lut[d3>>8&0xff]+lut[d3>>16&0xff]+lut[d3>>24&0xff];
}

exports.obs = obs;
exports.Test = CTest;
exports.TestGroup = CTestGroup;
exports.uuid = uuid;
