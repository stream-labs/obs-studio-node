"use strict";
Object.defineProperty(exports, "__esModule", { value: true });

const {obs} = require('./obs.js')
const {app} = require('electron')

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
				app.exit(1);
			}
			app.exit(0);
		} catch(e) {
			console.error("Uncaught exception: ", e);
			app.exit(-1);
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
			console.log("Running test " + name + "...");
			if ((this.tests[name] !== undefined) && (typeof(this.tests[name]) === "function")) {
				try {
					var result = await new Promise(this.tests[name]);
					if (result === false) {
						returnValue = 1;
						break;
					}
				} catch(e) {
					returnValue = -1;
					console.error("Uncaught exception: ", e);
					break;
				}
			}
		}
		if (this.fnFinalizer) {
			this.fnFinalizer();
		}
		app.exit(returnValue);
	}
}

exports.obs = obs;
exports.Test = CTest;
exports.TestGroup = CTestGroup;