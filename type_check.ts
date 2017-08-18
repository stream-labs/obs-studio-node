import * as obs from './module'

export function isNumberProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.INumberProperty {
    return type === obs.EPropertyType.Int ||
        type === obs.EPropertyType.Float;
}

export function isTextProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.ITextProperty {
    return type === obs.EPropertyType.Text;
}

export function isPathProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IPathProperty {
    return type === obs.EPropertyType.Path;
}

export function isListProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IListProperty {
    return type === obs.EPropertyType.List;
}

export function isEditableListProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IEditableListProperty {
    return type === obs.EPropertyType.EditableList;
}

export function isEmptyProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is {} {
    switch (type) {
    case obs.EPropertyType.Boolean:
    case obs.EPropertyType.Button:
    case obs.EPropertyType.Color:
    case obs.EPropertyType.Font:
    case obs.EPropertyType.Invalid:
        return true;
    }

    return false;
}