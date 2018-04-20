"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function isNumberProperty(property) {
    return property.type === 2 ||
        property.type === 3;
}
exports.isNumberProperty = isNumberProperty;
function isTextProperty(property) {
    return property.type === 4;
}
exports.isTextProperty = isTextProperty;
function isPathProperty(property) {
    return property.type === 5;
}
exports.isPathProperty = isPathProperty;
function isListProperty(property) {
    return property.type === 6;
}
exports.isListProperty = isListProperty;
function isEditableListProperty(property) {
    return property.type === 10;
}
exports.isEditableListProperty = isEditableListProperty;
function isBooleanProperty(property) {
    return property.type === 1;
}
exports.isBooleanProperty = isBooleanProperty;
function isButtonProperty(property) {
    return property.type === 8;
}
exports.isButtonProperty = isButtonProperty;
function isColorProperty(property) {
    return property.type === 7;
}
exports.isColorProperty = isColorProperty;
function isFontProperty(property) {
    return property.type === 9;
}
exports.isFontProperty = isFontProperty;
function isEmptyProperty(property) {
    switch (property.type) {
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
