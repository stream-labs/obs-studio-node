"use strict";

const {uuid, TestGroup} = require("../helpers/bootstrap.js")

let tg = new TestGroup();

tg.addTest("Generate Single UUID", (resolve, reject) => {
    let uuids = uuid();
    console.log("Generated Id: ", uuids);
    resolve(true);
    return;
});

let uuidTestLength = 100000;
tg.addTest("Generate "+uuidTestLength+" unique UUID", (resolve, reject) => {
    let uq = {};
    for (let i = 0; i < uuidTestLength; i++) {
        let uuids = uuid();
        if (uq[uuids] != undefined) {
            reject("Duplicate uuid found: " + uuids);
            return;
        }
        uq[uuids] = true;
    }
    resolve(true);
    return;
});

tg.run();
