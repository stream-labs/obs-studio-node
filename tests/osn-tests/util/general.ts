export function getCppErrorMsg(errorStack: any): string {
    return errorStack.stack.split("\n", 1).join("").substring(7);
}