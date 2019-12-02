export const enum ETestErrorMsg {
}

export function GetErrorMessage(message: string, value1?: string, value2?: string, value3?: string): string {
    let replacements: any;

    if (typeof value1 != 'undefined' &&
        typeof value2 != 'undefined' &&
        typeof value3 != 'undefined') {
        replacements = {"%VALUE1%": value1, "%VALUE2%": value2, "%VALUE3%": value3}
    } else if (typeof value1 != 'undefined' &&
               typeof value2 != 'undefined') {
        replacements = {"%VALUE1%": value1, "%VALUE2%": value2}
    } else if (typeof value1 != 'undefined') {
        replacements = {"%VALUE1%": value1}
    } else {
        return message;
    }

    let errorMessage = message;

    errorMessage = errorMessage.replace(/%\w+%/g, function(all) {
        return replacements[all] || all;
    });

    return errorMessage;
}