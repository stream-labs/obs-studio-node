const {obs, Test} = require("../helpers/bootstrap.js")

let self = new Test();
self.test = function(resolve, reject) {
	// Addition 
	{
		if ((1 + 1) != 2) {
			console.error("1 + 1 did not equal 2");
			resolve(false);
		}
		
		let var_1 = 1
		if ((var_1 + var_1) != 2) {
			console.error("var_1 + var_1 did not equal 2");
			resolve(false);
		}
		
		let var_2 = 2
		if ((var_1 + var_2) != 3) {
			console.error("var_1 + var_2 did not equal 3");
			resolve(false);
		}
	}
	
	// Substraction 
	{
		if ((1 - 1) != 0) {
			console.error("1 - 1 did not equal 0");
			resolve(false);
		}
		
		let var_1 = 1
		if ((var_1 - var_1) != 0) {
			console.error("var_1 - var_1 did not equal 0");
			resolve(false);
		}
		
		let var_2 = 2
		if ((var_1 - var_2) != -1) {
			console.error("var_1 - var_2 did not equal -1");
			resolve(false);
		}
	}
	
	// Multiplication 
	{
		if ((1 * 1) != 1) {
			console.error("1 * 1 did not equal 1");
			resolve(false);
		}
		
		let var_1 = 1
		if ((var_1 * var_1) != 1) {
			console.error("var_1 * var_1 did not equal 1");
			resolve(false);
		}
		
		let var_2 = 2
		if ((var_1 * var_2) != 2) {
			console.error("var_1 * var_2 did not equal 2");
			resolve(false);
		}
	}

	// Division 
	{
		if ((1 / 1) != 1) {
			console.error("1 / 1 did not equal 1");
			resolve(false);
		}
		
		let var_1 = 1
		if ((var_1 / var_1) != 1) {
			console.error("var_1 / var_1 did not equal 1");
			resolve(false);
		}
		
		let var_2 = 2
		if ((var_1 / var_2) != 0.5) {
			console.error("var_1 / var_2 did not equal 0.5");
			resolve(false);
		}
	}
	
	resolve(true);
}
self.run();
