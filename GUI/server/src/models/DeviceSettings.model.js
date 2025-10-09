const db = require('../helper/db.helper');

// CREATE TABLE DeviceSettings (
//     DeviceSettingId INTEGER PRIMARY KEY,
//     DeviceMaxUser INTEGER NOT NULL DEFAULT 6,
//     DeviceAutoDeleteLogThreshold INTEGER NOT NULL, 
//     DeviceAutoDeleteLogEnable BOOLEAN NOT NULL DEFAULT 1,
//     DeviceAutoRotationLogs BOOLEAN NOT NULL DEFAULT 1,
//     DeviceAutoDeleteActivityInterval INTEGER NOT NULL,
//     DeviceAutoDeleteActivity BOOLEAN NOT NULL DEFAULT 1
// );

const getDeviceSettings = () => {
    const sql = `SELECT * FROM DeviceSettings WHERE DeviceSettingId = 1`;
    return new Promise((resolve, reject) => {
        db.get(sql, [], (err, row) => {
            if (err) {
                reject(err);
            } else {
                resolve(row);
            }
        });
    });
};

const updateDeviceSettings = (deviceSettings) => {
    const sql = `UPDATE DeviceSettings SET DeviceMaxUser = ?, DeviceAutoDeleteLogThreshold = ?, DeviceAutoDeleteLogEnable = ?, DeviceAutoRotationLogs = ?, DeviceAutoDeleteActivityInterval = ?, DeviceAutoDeleteActivity = ? WHERE DeviceSettingId = 1`;
    return new Promise((resolve, reject) => {
        db.run(sql, [deviceSettings.DeviceMaxUser, deviceSettings.DeviceAutoDeleteLogThreshold, deviceSettings.DeviceAutoDeleteLogEnable, deviceSettings.DeviceAutoRotationLogs, deviceSettings.DeviceAutoDeleteActivityInterval, deviceSettings.DeviceAutoDeleteActivity], function (err) {
            if (err) {
                reject(err);
            } else {
                resolve({ id: this.lastID });
            }
        });
    });
}

const updateDeviceDiskSetting = (DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs) => { 
    const sql = `UPDATE DeviceSettings SET DeviceAutoDeleteLogEnable = ?, DeviceAutoDeleteLogThreshold = ?, DeviceAutoRotationLogs = ? WHERE DeviceSettingId = 1`;
    return new Promise((resolve, reject) => {
        db.run(sql, [DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs], function (err) {
            if (err) {
                reject(err);
            } else {
                resolve({ id: this.lastID });
            }
        });
    });
}

module.exports = { 
    getDeviceSettings,
    updateDeviceSettings,
    updateDeviceDiskSetting
};