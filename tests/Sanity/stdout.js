const {obs, Test} = require("../helpers/bootstrap.js")

let self = new Test();
self.test = function(resolve, reject) {
	console.log("This test is meant to test the output capabilities of Node.js and the proper displaying of it in the test runner.");
	console.log("It should show two separate lines with its own content each under standard output.");
	resolve(true);
}
self.run();
