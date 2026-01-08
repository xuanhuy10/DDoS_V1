const db = require("../helper/db.helper");

// create table DeviceInterfaces
// (
//     InterfaceId                   INTEGER
// primary key,
//     DefenseProfileId              INTEGER
// references DefenseProfiles,
//     InterfaceName                 TEXT                   not null,
//     InterfaceType                 TEXT                   not null,
//     InterfaceStatus               TEXT    default 'Up'   not null,
//     InterfaceDescription          TEXT,
//     InterfaceProtectionMode       TEXT    default 'Port' not null,
//     InterfaceIsMonitoring         BOOLEAN default 0      not null,
//     InterfaceIsMirroring          BOOLEAN default 0      not null,
//     InterfaceToMonitorInterfaceId INTEGER
// references DeviceInterfaces,
//     InterfaceMirrorSetting        TEXT,
//     MirrorType                    TEXT,
//     Value
// );
//

const getAllDeviceInterfaces = () => {
  const sql = `SELECT * FROM DeviceInterfaces LEFT JOIN DefenseProfiles ON DeviceInterfaces.DefenseProfileId = DefenseProfiles.DefenseProfileId`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

const getMirroringDeviceInterfaces = () => {
  const sql = `
        SELECT 
            di.InterfaceId AS MirrorInterfaceId,
            di.InterfaceName AS MirrorInterfaceName,
            di.InterfaceType,
            di.InterfaceStatus AS MirrorInterfaceStatus,
            di.InterfaceDescription,
            di.InterfaceProtectionMode,
            di.InterfaceIsMonitoring,
            di.InterfaceIsMirroring,
            di.InterfaceToMonitorInterfaceId,
            di.InterfaceMirrorSetting AS MirrorSetting,
            di.MirrorType,
            di.Value, -- Thêm dòng này để lấy trường Value

            mi.InterfaceId AS MonitorInterfaceId,
            mi.InterfaceName AS MonitorInterfaceName,
            mi.InterfaceStatus AS MonitorInterfaceStatus
        FROM 
            DeviceInterfaces di
        LEFT JOIN
            DeviceInterfaces mi ON di.InterfaceToMonitorInterfaceId = mi.InterfaceId
        WHERE 
            di.InterfaceIsMirroring = 1
        AND di.InterfaceToMonitorInterfaceId IS NOT NULL
    `;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

const getDeviceInterfaceByMonitorInterfaceId = (monitorInterfaceId) => {
  const sql = `SELECT * FROM DeviceInterfaces WHERE InterfaceToMonitorInterfaceId = ?`;
  return new Promise((resolve, reject) => {
    db.get(sql, [monitorInterfaceId], (err, row) => {
      if (err) {
        reject(err);
      }
      resolve(row);
    });
  });
};

const getDeviceInterfaceByInterfaceId = (InterfaceId) => {
  const sql = `SELECT * FROM DeviceInterfaces WHERE InterfaceId = ?`;
  return new Promise((resolve, reject) => {
    db.get(sql, [InterfaceId], (err, row) => {
      if (err) {
        reject(err);
      }
      resolve(row);
    });
  });
};

const insertDeviceInterface = (deviceInterface) => {
  const sql = `INSERT INTO DeviceInterfaces (DefenseProfileId, InterfaceName, InterfaceType, InterfaceStatus, InterfaceDescription, InterfaceProtectionMode, InterfaceIsMonitoring, InterfaceIsMirroring, InterfaceToMonitorInterfaceId, InterfaceMirrorSetting, MirrorType, Value) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
  return new Promise((resolve, reject) => {
    db.run(
      sql,
      [
        deviceInterface.DefenseProfileId,
        deviceInterface.InterfaceName,
        deviceInterface.InterfaceType,
        deviceInterface.InterfaceStatus,
        deviceInterface.InterfaceDescription,
        deviceInterface.InterfaceProtectionMode,
        deviceInterface.InterfaceIsMonitoring,
        deviceInterface.InterfaceIsMirroring,
        deviceInterface.InterfaceToMonitorInterfaceId,
        deviceInterface.InterfaceMirrorSetting,
        deviceInterface.MirrorType, // <-- thêm dòng này
        deviceInterface.Value, // <-- thêm dòng này
      ],
      function (err) {
        if (err) {
          reject(err);
        }
        resolve({ id: this.lastID });
      }
    );
  });
};

function updateDeviceInterface(interfaceId, data) {
  const sql = `
    UPDATE DeviceInterfaces SET
      DefenseProfileId = ?,
      InterfaceName = ?,
      InterfaceType = ?,
      InterfaceStatus = ?,
      InterfaceDescription = ?,
      InterfaceProtectionMode = ?,
      InterfaceIsMonitoring = ?,
      InterfaceIsMirroring = ?,
      InterfaceToMonitorInterfaceId = ?,
      InterfaceMirrorSetting = ?,
      MirrorType = ?,
      Value = ?
    WHERE InterfaceId = ?
  `;
  const params = [
    data.DefenseProfileId,
    data.InterfaceName,
    data.InterfaceType,
    data.InterfaceStatus,
    data.InterfaceDescription,
    data.InterfaceProtectionMode,
    data.InterfaceIsMonitoring,
    data.InterfaceIsMirroring,
    data.InterfaceToMonitorInterfaceId,
    data.InterfaceMirrorSetting,
    data.MirrorType,
    data.Value,
    interfaceId,
  ];
  return db.run(sql, params);
}

const updateAllDeviceInterfacesToInbound = () => {
  const sql = `UPDATE DeviceInterfaces SET InterfaceType = 'inbound'`;
  return new Promise((resolve, reject) => {
    db.run(sql, [], function (err) {
      if (err) {
        reject(err);
      }
      resolve({ changes: this.changes });
    });
  });
};

const updateAllDeviceInterfacesDefenseProfileId = (defenseProfileId) => {
  const sql = `UPDATE DeviceInterfaces SET DefenseProfileId = ?`;
  return new Promise((resolve, reject) => {
    db.run(sql, [defenseProfileId], function (err) {
      if (err) {
        reject(err);
      }
      resolve({ changes: this.changes });
    });
  });
};

const deleteDeviceInterface = (deviceInterfaceId) => {
  const sql = `DELETE FROM DeviceInterfaces WHERE InterfaceId = ?`;
  return new Promise((resolve, reject) => {
    db.run(sql, [deviceInterfaceId], function (err) {
      if (err) {
        reject(err);
      }
      resolve({ id: this.lastID });
    });
  });
};

const checkInterfaceId = (interfaceName) => {
  if (!interfaceName) {
    return Promise.reject(new Error("InterfaceName is required"));
  }
  const sql = `SELECT InterfaceId FROM DeviceInterfaces WHERE InterfaceName = ?`;
  return new Promise((resolve, reject) => {
    db.get(sql, [interfaceName], (err, row) => {
      if (err) {
        reject(err);
      } else if (!row) {
        reject(new Error(`Interface with name '${interfaceName}' does not exist`));
      } else {
        resolve(row.InterfaceId);
      }
    });
  });
};

const disableOtherProfiles = (interfaceId) => {
  if (!interfaceId) {
    return Promise.reject(new Error("InterfaceId is required"));
  }
  const sql = `
    UPDATE IPSecProfiles 
    SET Enable = 0 
    WHERE Enable = 1 
    AND IPSecProfileId IN (
      SELECT IPSecProfileId FROM DeviceInterfaces WHERE InterfaceId = ?
    )
  `;
  return new Promise((resolve, reject) => {
    db.run(sql, [interfaceId], function (err) {
      if (err) {
        reject(err);
      } else {
        resolve({ changes: this.changes });
      }
    });
  });
};

module.exports = {
  getAllDeviceInterfaces,
  getMirroringDeviceInterfaces,
  getDeviceInterfaceByMonitorInterfaceId,
  getDeviceInterfaceByInterfaceId,
  insertDeviceInterface,
  updateDeviceInterface,
  updateAllDeviceInterfacesToInbound,
  updateAllDeviceInterfacesDefenseProfileId,
  deleteDeviceInterface,
  checkInterfaceId,
  disableOtherProfiles,
};
