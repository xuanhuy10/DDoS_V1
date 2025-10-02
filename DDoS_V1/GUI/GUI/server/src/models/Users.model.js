const db = require('../helper/db.helper');

// CREATE TABLE Users (
//     UserId INTEGER PRIMARY KEY,
//     UserFullName TEXT NOT NULL,
//     Username TEXT NOT NULL,
//     Password TEXT NOT NULL,
//     Role TEXT NOT NULL,
//     LastLogin DATETIME,
//     CreateTime DATETIME NOT NULL,
//     Email TEXT NOT NULL,
//     NotifyNetworkAnomalyDetect BOOLEAN NOT NULL DEFAULT 1,
//     NotifyDDoSAttackDetect BOOLEAN NOT NULL DEFAULT 1,
//     NotifyDDoSAttackEnd BOOLEAN NOT NULL DEFAULT 1,
//     NotifyDiskExceeds BOOLEAN NOT NULL DEFAULT 1
// );

const getAllUsers = () => {
    const sql = `SELECT UserId, UserFullName, Username, Role, LastLogin, CreateTime, Email, NotifyNetworkAnomalyDetect, NotifyDDoSAttackDetect, NotifyDDoSAttackEnd, NotifyDiskExceeds FROM Users ORDER BY CreateTime ASC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getUserByUserId = (UserId) => {
    const sql = `SELECT * FROM Users WHERE UserId = ?`;
    return new Promise((resolve, reject) => {
        db.get(sql, [UserId], (err, row) => {
            if (err) {
                reject(err);
            }
            resolve(row);
        });
    });
}

const getUserByUsername = (Username) => {
    const sql = `SELECT * FROM Users WHERE Username = ?`;
    return new Promise((resolve, reject) => {
        db.get(sql, [Username], (err, row) => {
            if (err) {
                reject(err);
            }
            resolve(row);
        });
    });
}

const getUserByEmail = (Email) => {
    const sql = `SELECT * FROM Users WHERE Email = ?`;
    return new Promise((resolve, reject) => {
        db.get(sql, [Email], (err, row) => {
            if (err) {
                reject(err);
            }
            resolve(row);
        });
    });
}

const insertUser = (user) => {
    const sql = `INSERT INTO Users (UserFullName, Username, Password, Role, LastLogin, CreateTime, Email, NotifyNetworkAnomalyDetect, NotifyDDoSAttackDetect, NotifyDDoSAttackEnd, NotifyDiskExceeds) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
    return new Promise((resolve, reject) => {
        db.run(sql, [user.UserFullName, user.Username, user.Password, user.Role, user.LastLogin, user.CreateTime, user.Email, user.NotifyNetworkAnomalyDetect, user.NotifyDDoSAttackDetect, user.NotifyDDoSAttackEnd, user.NotifyDiskExceeds], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const updateUser = (UserId, user) => {
    const sql = `UPDATE Users SET UserFullName = ?, Username = ?, Password = ?, Role = ?, LastLogin = ?, CreateTime = ?, Email = ?, NotifyNetworkAnomalyDetect = ?, NotifyDDoSAttackDetect = ?, NotifyDDoSAttackEnd = ?, NotifyDiskExceeds = ? WHERE UserId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [user.UserFullName, user.Username, user.Password, user.Role, user.LastLogin, user.CreateTime, user.Email, user.NotifyNetworkAnomalyDetect, user.NotifyDDoSAttackDetect, user.NotifyDDoSAttackEnd, user.NotifyDiskExceeds, UserId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteUser = (UserId) => {
    const sql = `DELETE FROM Users WHERE UserId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [UserId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteUserByUsername = (Username) => {
    const sql = `DELETE FROM Users WHERE Username = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [Username], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

module.exports = {
    getAllUsers,
    getUserByUserId,
    getUserByUsername,
    getUserByEmail,
    insertUser,
    updateUser,
    deleteUser,
    deleteUserByUsername
};