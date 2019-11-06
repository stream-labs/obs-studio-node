import { ISettings } from '../osn';

// Transition settings

let fadeToColor: ISettings = {
    color: 4278190080,
    switch_point: 50
};
export {fadeToColor};

let wipe: ISettings = {
    luma_image: 'linear-h.png',
    luma_invert: false,
    luma_softness: 0.03
};
export {wipe};