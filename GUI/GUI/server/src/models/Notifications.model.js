const db = require('../helper/dbLite.helper');

// CREATE TABLE Notifications (
//     NotificationId INTEGER PRIMARY KEY,
//     UserId INTEGER,
//     NotificationType TEXT NOT NULL,
//     NotificationContent TEXT NOT NULL,
//     NotificationTime DATETIME NOT NULL,
//     NotificationIsRead BOOLEAN NOT NULL DEFAULT 1,
//     FOREIGN KEY (UserId) REFERENCES User (UserId)
// );


const getAllNotifications = () => {
    const sql = `SELECT * FROM Notifications LEFT JOIN User ON Notifications.UserId = User.UserId ORDER BY NotificationTime DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getNotificationByNotificationId = (NotificationId) => {
    const sql = `SELECT * FROM Notifications WHERE NotificationId = ?`;
    return new Promise((resolve, reject) => {
        db.all(sql, [NotificationId], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getNotificationByUserId = (UserId) => {
    const sql = `SELECT * FROM Notifications WHERE UserId = ?`;
    return new Promise((resolve, reject) => {
        db.all(sql, [UserId], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getUnreadNotificationsByUserId = (UserId) => {
    const sql = `SELECT * FROM Notifications WHERE UserId = ? AND NotificationIsRead = 0`;
    return new Promise((resolve, reject) => {
        db.all(sql, [UserId], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const insertNotification = (notification) => {
    const sql = `INSERT INTO Notifications (UserId, NotificationType, NotificationContent, NotificationTime, NotificationIsRead) VALUES (?, ?, ?, ?, ?)`;
    return new Promise((resolve, reject) => {
        db.run(sql, [notification.UserId, notification.NotificationType, notification.NotificationContent, notification.NotificationTime, notification.NotificationIsRead], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const updateNotification = (NotificationId, notification) => {
    const sql = `UPDATE Notifications SET UserId = ?, NotificationType = ?, NotificationContent = ?, NotificationTime = ?, NotificationIsRead = ? WHERE NotificationId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [notification.UserId, notification.NotificationType, notification.NotificationContent, notification.NotificationTime, notification.NotificationIsRead, NotificationId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteNotification = (NotificationId) => {
    const sql = `DELETE FROM Notifications WHERE NotificationId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [NotificationId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteAllNotificationsByUserId = (UserId) => {
    const sql = `DELETE FROM Notifications`;
    return new Promise((resolve, reject) => {
        db.run(sql, [], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteOldNotifications = () => {
    const sql = `DELETE FROM Notifications WHERE NotificationTime < datetime('now', '-30 days')`;
    return new Promise((resolve, reject) => {
        db.run(sql, [], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const markAllNotificationsAsRead = (UserId) => {
    const sql = `UPDATE Notifications SET NotificationIsRead = 1 WHERE UserId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [UserId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const markNotificationAsRead = (NotificationId) => {
    const sql = `UPDATE Notifications SET NotificationIsRead = 1 WHERE NotificationId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [NotificationId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}



