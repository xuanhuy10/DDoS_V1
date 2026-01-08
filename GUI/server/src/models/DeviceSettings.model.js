const db = require('../helper/db.helper');

// create table DeviceSettings
// (
//     DeviceSettingId                  INTEGER
// primary key,
//     DeviceMaxUser                    INTEGER default 6 not null,
//     DeviceAutoDeleteLogThreshold     INTEGER           not null,
//     DeviceAutoDeleteLogEnable        BOOLEAN default 1 not null,
//     DeviceAutoRotationLogs           BOOLEAN default 1 not null,
//     DeviceAutoDeleteActivityInterval INTEGER           not null,
//     DeviceAutoDeleteActivity         BOOLEAN default 1 not null
// );
//


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