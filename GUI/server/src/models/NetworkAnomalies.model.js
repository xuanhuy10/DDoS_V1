const db = require('../helper/db.helper');

// CREATE TABLE NetworkAnomalies (
//     AnomaliesId INTEGER PRIMARY KEY,
//     Anomalies TEXT NOT NULL,
//     AnomaliesTargetIp TEXT NOT NULL,
//     AnomaliesTargetPort INTEGER NOT NULL,
//     AnomaliesStats TEXT NOT NULL,
//     AnomaliesStart DATETIME NOT NULL,
//     AnomaliesEnd DATETIME,
//     AnomaliesStatus BOOLEAN NOT NULL DEFAULT 1
// );

const getAllNetworkAnomalies = () => {
    const sql = `SELECT * FROM NetworkAnomalies ORDER BY AnomaliesStart DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getNetworkAnomaliesByAnomaliesId = (AnomaliesId) => {
    const sql = `SELECT * FROM NetworkAnomalies WHERE AnomaliesId = ?`;
    return new Promise((resolve, reject) => {
        db.get(sql, [AnomaliesId], (err, row) => {
            if (err) {
                reject(err);
            }
            resolve(row);
        });
    });
}

const getNetworkAnomaliesByAnomaliesStatus = (AnomaliesStatus) => {
    const sql = `SELECT * FROM NetworkAnomalies WHERE AnomaliesStatus = ?`;
    return new Promise((resolve, reject) => {
        db.all(sql, [AnomaliesStatus], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const insertNetworkAnomalies = (networkAnomalies) => {
    const sql = `INSERT INTO NetworkAnomalies (Anomalies, AnomaliesTargetIp, AnomaliesTargetPort, AnomaliesStats, AnomaliesStart, AnomaliesEnd, AnomaliesStatus) VALUES (?, ?, ?, ?, ?, ?, ?)`;
    return new Promise((resolve, reject) => {
        db.run(sql, [networkAnomalies.Anomalies, networkAnomalies.AnomaliesTargetIp, networkAnomalies.AnomaliesTargetPort, networkAnomalies.AnomaliesStats, networkAnomalies.AnomaliesStart, networkAnomalies.AnomaliesEnd, networkAnomalies.AnomaliesStatus], function (err) {
            if (err) {
                reject(err);
            }
            resolve(this.lastID);
        });
    });
}

const updateNetworkAnomalies = (AnomaliesId, networkAnomalies) => {
    const sql = `UPDATE NetworkAnomalies SET Anomalies = ?, AnomaliesTargetIp = ?, AnomaliesTargetPort = ?, AnomaliesStats = ?, AnomaliesStart = ?, AnomaliesEnd = ?, AnomaliesStatus = ? WHERE AnomaliesId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [networkAnomalies.Anomalies, networkAnomalies.AnomaliesTargetIp, networkAnomalies.AnomaliesTargetPort, networkAnomalies.AnomaliesStats, networkAnomalies.AnomaliesStart, networkAnomalies.AnomaliesEnd, networkAnomalies.AnomaliesStatus, AnomaliesId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

module.exports = {
    getAllNetworkAnomalies,
    getNetworkAnomaliesByAnomaliesId,
    getNetworkAnomaliesByAnomaliesStatus,
    insertNetworkAnomalies,
    updateNetworkAnomalies
}