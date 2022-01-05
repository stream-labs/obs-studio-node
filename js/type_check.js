"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isEmptyProperty = exports.isFontProperty = exports.isCaptureProperty = exports.isColorProperty = exports.isButtonProperty = exports.isBooleanProperty = exports.isEditableListProperty = exports.isListProperty = exports.isPathProperty = exports.isTextProperty = exports.isNumberProperty = void 0;
function isNumberProperty(property) {
    return property.type === "OBS_PROPERTY_INT" ||
        property.type === "OBS_PROPERTY_FLOAT";
}
exports.isNumberProperty = isNumberProperty;
function isTextProperty(property) {
    return property.type === "OBS_PROPERTY_TEXT";
}
exports.isTextProperty = isTextProperty;
function isPathProperty(property) {
    return property.type === "OBS_PROPERTY_FILE";
}
exports.isPathProperty = isPathProperty;
function isListProperty(property) {
    return property.type === "OBS_PROPERTY_LIST";
}
exports.isListProperty = isListProperty;
function isEditableListProperty(property) {
    return property.type === "OBS_PROPERTY_EDITABLE_LIST";
}
exports.isEditableListProperty = isEditableListProperty;
function isBooleanProperty(property) {
    return property.type === "OBS_PROPERTY_BOOL";
}
exports.isBooleanProperty = isBooleanProperty;
function isButtonProperty(property) {
    return property.type === "OBS_PROPERTY_BUTTON";
}
exports.isButtonProperty = isButtonProperty;
function isColorProperty(property) {
    return property.type === "OBS_PROPERTY_COLOR";
}
exports.isColorProperty = isColorProperty;
function isCaptureProperty(property) {
    return property.type === "OBS_PROPERTY_CAPTURE";
}
exports.isCaptureProperty = isCaptureProperty;
function isFontProperty(property) {
    return property.type === "OBS_PROPERTY_FONT";
}
exports.isFontProperty = isFontProperty;
function isEmptyProperty(property) {
    switch (property.type) {
        case "OBS_PROPERTY_BOOL":
        case "OBS_PROPERTY_BUTTON":
        case "OBS_PROPERTY_COLOR":
        case "OBS_PROPERTY_FONT":
        case "OBS_PROPERTY_INVALID":
            return true;
    }
    return false;
}
exports.isEmptyProperty = isEmptyProperty;
