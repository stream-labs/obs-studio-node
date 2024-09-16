"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isEmptyProperty = exports.isFontProperty = exports.isCaptureProperty = exports.isColorProperty = exports.isButtonProperty = exports.isBooleanProperty = exports.isEditableListProperty = exports.isListProperty = exports.isPathProperty = exports.isTextProperty = exports.isNumberProperty = void 0;
const obs = require("./module");
function isNumberProperty(property) {
    return property.type === obs.EPropertyType.Int ||
        property.type === obs.EPropertyType.Float;
}
exports.isNumberProperty = isNumberProperty;
function isTextProperty(property) {
    return property.type === obs.EPropertyType.Text;
}
exports.isTextProperty = isTextProperty;
function isPathProperty(property) {
    return property.type === obs.EPropertyType.Path;
}
exports.isPathProperty = isPathProperty;
function isListProperty(property) {
    return property.type === obs.EPropertyType.List;
}
exports.isListProperty = isListProperty;
function isEditableListProperty(property) {
    return property.type === obs.EPropertyType.EditableList;
}
exports.isEditableListProperty = isEditableListProperty;
function isBooleanProperty(property) {
    return property.type === obs.EPropertyType.Boolean;
}
exports.isBooleanProperty = isBooleanProperty;
function isButtonProperty(property) {
    return property.type === obs.EPropertyType.Button;
}
exports.isButtonProperty = isButtonProperty;
function isColorProperty(property) {
    return property.type === obs.EPropertyType.Color;
}
exports.isColorProperty = isColorProperty;
function isCaptureProperty(property) {
    return property.type === obs.EPropertyType.Capture;
}
exports.isCaptureProperty = isCaptureProperty;
function isFontProperty(property) {
    return property.type === obs.EPropertyType.Font;
}
exports.isFontProperty = isFontProperty;
function isEmptyProperty(property) {
    switch (property.type) {
        case obs.EPropertyType.Boolean:
        case obs.EPropertyType.Button:
        case obs.EPropertyType.Color:
        case obs.EPropertyType.Font:
        case obs.EPropertyType.Invalid:
            return true;
    }
    return false;
}
exports.isEmptyProperty = isEmptyProperty;
