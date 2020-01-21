export function logInfo(testName: string, message: string) {
    console.log('%s: [INFO] %s', testName, message);
}

export function logWarning(testName: string, message: string) {
    console.log('%s: [WARNING] %s', testName, message);
}

export function logWithNoTestName(message: string) {
    console.log(message);
}

export function logEmptyLine() {
    console.log('');
}