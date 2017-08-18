import * as obs from './module';
export declare function isNumberProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.INumberProperty;
export declare function isTextProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.ITextProperty;
export declare function isPathProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IPathProperty;
export declare function isListProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IListProperty;
export declare function isEditableListProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is obs.IEditableListProperty;
export declare function isEmptyProperty(details: obs.TPropertyDetails, type: obs.EPropertyType): details is {};
