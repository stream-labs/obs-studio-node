var mocha = require('mocha');

function ListReporter(runner) {
    mocha.reporters.Base.call(this, runner);
    var passes = 0;
    var failures = 0;
    var failedTestCases = [];
    var testLine = "";

    runner.on('start', function() {
        console.log('');
    });

    runner.on('pass', function(test) {
        passes++;
        console.log('%s: [TEST CASE] %s [PASS] %dms', test.parent.title, test.title, test.duration);
    });

    runner.on('fail', function(test, err) {
        failures++;

        // Getting test line with the expect check that failed
        var regex = /(?<=src\\)(.*?)(?=\n)/;
        var str = err.stack.match(regex);
        if (str != null) {
            testLine = str[0].substr(0, str[0].lastIndexOf(":"));
        }

        // Formatting failure message
        var testCaseFailMsg = test.parent.title + ': [TEST CASE] ' + test.title + ' | [FAIL] ' + testLine;

        console.log(testCaseFailMsg);
        console.log('%s: [FAIL REPORT] Error - %s', test.parent.title, err.message);

        failedTestCases.push(testCaseFailMsg);
    });

    runner.on('end', function() {
        console.log('%d passing', passes);

        if (failures >= 1) {
            console.log('%d failing', failures);

            failedTestCases.forEach(testCase => {
                console.log('  - %s', testCase);
            });
            
            console.log('');
        }
    });
}

module.exports = ListReporter;