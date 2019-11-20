var mocha = require('mocha');
module.exports = ListReporter;

function ListReporter(runner) {
    mocha.reporters.Base.call(this, runner);
    var passes = 0;
    var failures = 0;

    runner.on('start', function() {
        console.log('');
    });

    runner.on('pass', function(test) {
        passes++;
        console.log('%s: %s pass %dms', test.parent.title, test.title, test.duration);
    });

    runner.on('fail', function(test, err) {
        failures++;
        console.log('fail: %s -- error: %s', test.fullTitle(), err.message);
    });

    runner.on('end', function() {
        console.log('%d passing', passes);

        if (failures >= 1) {
            console.log('%d failing', failures);
        }
    });
}