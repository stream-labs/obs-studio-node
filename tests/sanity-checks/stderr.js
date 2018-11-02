const {Test} = require("../helpers/bootstrap.js")

let self = new Test();
self.test = function(resolve, reject) {
	console.error("This test is meant to test the output capabilities of Node.js and the proper displaying of it in the test runner.");
	console.error("It should show two separate lines with its own content each under standard error.");
	resolve(true);
}
self.run();
