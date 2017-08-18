"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const obs = require("./module");
function isNumberProperty(details, type) {
    return type === 2 ||
        type === 3;
}
exports.isNumberProperty = isNumberProperty;
function isTextProperty(details, type) {
    return type === 4;
}
exports.isTextProperty = isTextProperty;
function isPathProperty(details, type) {
    return type === 5;
}
exports.isPathProperty = isPathProperty;
function isListProperty(details, type) {
    return type === 6;
}
exports.isListProperty = isListProperty;
function isEditableListProperty(details, type) {
    return type === 10;
}
exports.isEditableListProperty = isEditableListProperty;
function isEmptyProperty(details, type) {
    switch (type) {
        case 1:
        case 8:
        case 7:
        case 9:
        case 0:
            return true;
    }
    return false;
}
exports.isEmptyProperty = isEmptyProperty;
