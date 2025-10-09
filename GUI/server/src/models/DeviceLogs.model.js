const db = require('../helper/db.helper');

const getAllDeviceLogs = () => {
    const sql = `SELECT * FROM DeviceLogs LEFT JOIN Users ON DeviceLogs.UserId = Users.UserId ORDER BY LogActionTime DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getDeviceLogByLogId = (LogId) => {
    const sql = `SELECT * FROM DeviceLogs WHERE LogId = ?`;
    return new Promise((resolve, reject) => {
        db.all(sql, [LogId], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getDeviceLogByLogType = (LogType) => {
    const sql = `SELECT * FROM DeviceLogs LEFT JOIN Users ON DeviceLogs.UserId = Users.UserId  WHERE LogType = ? ORDER BY LogActionTime DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [LogType], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const insertDeviceLog = (deviceLog) => {
    const sql = `INSERT INTO DeviceLogs (UserId, LogSource, LogType, LogActionTime, LogActionContent, LogActionDetail, LogActionResult, LogActionResultDetail) VALUES (?, ?, ?, ?, ?, ?, ?, ?)`;
    return new Promise((resolve, reject) => {
        db.run(sql, [deviceLog.UserId, deviceLog.LogSource, deviceLog.LogType, deviceLog.LogActionTime, deviceLog.LogActionContent, deviceLog.LogActionDetail, deviceLog.LogActionResult, deviceLog.LogActionResultDetail], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const updateDeviceLog = (LogId, deviceLog) => {
    const sql = `UPDATE DeviceLogs SET UserId = ?, LogSource = ?, LogType = ?, LogActionTime = ?, LogActionContent = ?, LogActionDetail = ?, LogActionResult = ?, LogActionResultDetail = ? WHERE LogId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [deviceLog.UserId, deviceLog.LogSource, deviceLog.LogType, deviceLog.LogActionTime, deviceLog.LogActionContent, deviceLog.LogActionDetail, deviceLog.LogActionResult, deviceLog.LogActionResultDetail, LogId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteDeviceLog = (LogId) => {
    const sql = `DELETE FROM DeviceLogs WHERE LogId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [LogId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteAllDeviceLogs = () => {
    const sql = `DELETE FROM DeviceLogs`;
    return new Promise((resolve, reject) => {
        db.run(sql, [], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

// Xóa nhiều logs theo id
const deleteDeviceLogsByIds = (logType, ids) => {
    const sql = `DELETE FROM DeviceLogs WHERE LogType = ? AND LogId IN (${ids.map(() => '?').join(',')})`;
    return new Promise((resolve, reject) => {
        db.run(sql, [logType, ...ids], function (err) {
            if (err) return reject(err);
            console.log('Deleted rows:', this.changes);
            resolve({ changes: this.changes });
        });
    });
};  

// Lấy logs theo id (để export)
const getDeviceLogsByIds = (logType, ids) => {
    const sql = `SELECT * FROM DeviceLogs WHERE LogType = ? AND LogId IN (${ids.map(() => '?').join(',')})`;
    return new Promise((resolve, reject) => {
        db.all(sql, [logType, ...ids], (err, rows) => {
            if (err) reject(err);
            resolve(rows);
        });
    });
};

const getDeviceLogsByTypeAndTimeRange = (logType, from, to) => {
    const sql = `
        SELECT * FROM DeviceLogs
        WHERE LogType = ? AND LogActionTime BETWEEN ? AND ?
        ORDER BY LogActionTime DESC
    `;
    return new Promise((resolve, reject) => {
        db.all(sql, [logType, from, to], (err, rows) => {
            if (err) reject(err);
            resolve(rows);
        });
    });
};

const deleteDeviceLogsByTypeAndTimeRange = (logType, from, to) => {
    const sql = `
        DELETE FROM DeviceLogs 
        WHERE LogType = ? AND LogActionTime >= ? AND LogActionTime <= ?
    `;
    return new Promise((resolve, reject) => {
        db.run(sql, [logType, from, to], function (err) {
            if (err) return reject(err);
            resolve({ changes: this.changes });
        });
    });
};

const deleteOldActivityLogs = (days) => {
    const now = new Date();
    const thresholdDate = new Date(now.getTime() - days * 24 * 60 * 60 * 1000);

    // Format về 'YYYY/MM/DD HH:mm:ss'
    const pad = (n) => n.toString().padStart(2, '0');
    const formatted =
        thresholdDate.getFullYear() + '/' +
        pad(thresholdDate.getMonth() + 1) + '/' +
        pad(thresholdDate.getDate()) + ' ' +
        pad(thresholdDate.getHours()) + ':' +
        pad(thresholdDate.getMinutes()) + ':' +
        pad(thresholdDate.getSeconds());

    const sql = `DELETE FROM DeviceLogs WHERE LogActionTime < ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [formatted], function (err) {
            if (err) return reject(err);
            resolve({ changes: this.changes });
        });
    });
};

module.exports = {
    getAllDeviceLogs,
    getDeviceLogByLogId,
    getDeviceLogByLogType,
    insertDeviceLog,
    updateDeviceLog,
    deleteDeviceLog,
    deleteAllDeviceLogs,
    deleteDeviceLogsByIds,
    getDeviceLogsByIds,
    getDeviceLogsByTypeAndTimeRange,
    deleteDeviceLogsByTypeAndTimeRange,
    deleteOldActivityLogs,
};