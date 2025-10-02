const db = require('../helper/db.helper');

// CREATE TABLE NetworkAddresses (
//     AddressId INTEGER PRIMARY KEY,
//     InterfaceId INTEGER,
//     Address TEXT NOT NULL,
//     AddressVersion TEXT NOT NULL,
//     AddressType TEXT NOT NULL,
//     AddressAddedDate DATETIME NOT NULL,
//     AddressTimeOut DATETIME NOT NULL,
//     FOREIGN KEY (InterfaceId) REFERENCES DeviceInterfaces (InterfaceId)
// );

const getAllNetworkAddresses = () => {
    const sql = `SELECT * FROM NetworkAddresses ORDER BY AddressAddedDate DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const getNetworkAddressesByAddressId = (AddressId) => {
    const sql = `SELECT * FROM NetworkAddresses WHERE AddressId = ?`;
    return new Promise((resolve, reject) => {
        db.get(sql, [AddressId], (err, row) => {
            if (err) {
                reject(err);
            }
            resolve(row);
        });
    });
}

const getNetworkAddressesByAddressType = (AddressType) => {
    const sql = `SELECT * FROM NetworkAddresses LEFT JOIN DeviceInterfaces ON NetworkAddresses.InterfaceId = DeviceInterfaces.InterfaceId WHERE AddressType = ? ORDER BY AddressAddedDate DESC`;
    return new Promise((resolve, reject) => {
        db.all(sql, [AddressType], (err, rows) => {
            if (err) {
                reject(err);
            }
            resolve(rows);
        });
    });
}

const insertNetworkAddress = (networkAddress) => {
    const sql = `INSERT INTO NetworkAddresses (Address, AddressVersion, AddressType, InterfaceId, AddressAddedDate, AddressTimeOut, Port ) VALUES (?, ?, ?, ?, ?, ?, ?)`;
    return new Promise((resolve, reject) => {
        db.run(sql, [networkAddress.Address, networkAddress.AddressVersion, networkAddress.AddressType, networkAddress.InterfaceId, networkAddress.AddressAddedDate, networkAddress.AddressTimeOut, networkAddress.Port], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const updateNetworkAddress = (AddressId, networkAddress) => {
    const sql = `UPDATE NetworkAddresses SET Address = ?, AddressVersion = ?, AddressType = ?, InterfaceId = ?, AddressAddedDate = ?, AddressTimeOut = ? WHERE AddressId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [networkAddress.Address, networkAddress.AddressVersion, networkAddress.AddressType, networkAddress.InterfaceId, networkAddress.AddressAddedDate, networkAddress.AddressTimeOut, AddressId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteNetworkAddress = (AddressId) => {
    const sql = `DELETE FROM NetworkAddresses WHERE AddressId = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [AddressId], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ id: this.lastID });
        });
    });
}

const deleteNetworkAddressByAddressAndVersion = (address, version) => {
    const sql = `DELETE FROM NetworkAddresses WHERE Address = ? AND AddressVersion = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [address, version], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ changes: this.changes });
        });
    });
};

async function deleteNetworkAddressByAddressVersionAndPort(address, addressVersion, port) {
    const sql = `DELETE FROM NetworkAddresses WHERE Address = ? AND AddressVersion = ? AND Port = ?`;
    return new Promise((resolve, reject) => {
        db.run(sql, [address, addressVersion, port], function (err) {
            if (err) {
                reject(err);
            }
            resolve({ changes: this.changes });
        });
    });
}

module.exports = {
    getAllNetworkAddresses,
    getNetworkAddressesByAddressId,
    getNetworkAddressesByAddressType,
    insertNetworkAddress,
    updateNetworkAddress,
    deleteNetworkAddress,
    deleteNetworkAddressByAddressAndVersion,
    deleteNetworkAddressByAddressVersionAndPort
}