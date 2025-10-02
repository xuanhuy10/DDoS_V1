const db = require("../helper/db.helper");

// CREATE TABLE DefenseProfiles (
//     DefenseProfileId INTEGER PRIMARY KEY,
//     UserId INTEGER,
//     DefenseProfileName TEXT NOT NULL,
//     DefenseProfileDescription TEXT NOT NULL,
//     DefenseProfileCreateTime DATETIME NOT NULL,
//     DefenseProfileLastModified DATETIME  NOT NULL,
//     DefenseProfileUsingTime DATETIME,
//     DefenseProfileType TEXT NOT NULL,
//     DefenseProfileStatus TEXT NOT NULL,
//     DetectionTime INTEGER NOT NULL DEFAULT 1,
//     DefenseMode TEXT NOT NULL DEFAULT 'Aggregate',
//     ICMPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     ICMPFloodThreshold INTEGER NOT NULL DEFAULT 1000,
//     ICMPFloodRate INTEGER NOT NULL DEFAULT 1000,
//     SYNFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     SYNFloodSYNThreshold INTEGER NOT NULL DEFAULT 1000,
//     SYNFloodACKThreshold INTEGER NOT NULL DEFAULT 1000,
//     SYNFloodWhiteListTimeOut INTEGER NOT NULL DEFAULT 20,
//     UDPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     UDPFloodThreshold INTEGER NOT NULL DEFAULT 1000,
//     UDPFloodRate INTEGER NOT NULL DEFAULT 1000,
//     DNSFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     DNSFloodThreshold INTEGER NOT NULL DEFAULT 1000,
//     LandAttackEnable BOOLEAN NOT NULL DEFAULT 1,
//     IPSecIKEEnable BOOLEAN NOT NULL DEFAULT 1,
//     IPSecIKEThreshold INTEGER NOT NULL DEFAULT 1000,
//     TCPFragmentEnable BOOLEAN NOT NULL DEFAULT 1,
//     UDPFragmentEnable BOOLEAN NOT NULL DEFAULT 1,
//     HTTPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     HTTPSFloodEnable BOOLEAN NOT NULL DEFAULT 1,
//     FOREIGN KEY (UserId) REFERENCES User (UserId)
// );

const getAllDefenseProfiles = () => {
  const sql = `SELECT * FROM DefenseProfiles`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

const getAllDefenseProfilesName = () => {
  const sql = `SELECT DefenseProfileName FROM DefenseProfiles`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

const getAllDefenseProfilesbyUserId = (UserId) => {
  const sql = `SELECT * FROM DefenseProfiles WHERE UserId = ?`;
  return new Promise((resolve, reject) => {
    db.all(sql, [UserId], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

// WTF is this doing?
const getDefenseProfilesbyOffset = (offset) => {
  const sql = `SELECT * FROM DefenseProfiles ORDER BY DefenseProfileCreateTime DESC LIMIT 5 OFFSET ?`;
  return new Promise((resolve, reject) => {
    db.all(sql, [offset], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

const getDefenseProfilebyDefenseProfileId = (DefenseProfileId) => {
  const sql = `SELECT * FROM DefenseProfiles WHERE DefenseProfileId = ?`;
  return new Promise((resolve, reject) => {
    db.get(sql, [DefenseProfileId], (err, row) => {
      if (err) {
        reject(err);
      }
      resolve(row);
    });
  });
};

const getActiveDefenseProfile = () => {
  const sql = `
        SELECT *
        FROM DefenseProfiles LEFT OUTER JOIN Users ON DefenseProfiles.UserId = Users.UserId
        WHERE DefenseProfileStatus = 'Active'`;
  return new Promise((resolve, reject) => {
    db.all(sql, (err, rows) => {
      if (err) {
        reject(err);
      } else {
        resolve(rows);
      }
    });
  });
};

const getAttackTypeOfDefenseProfile = (attack_type) => {
  let sql = "";
  // Xử lý điều kiện cho từng case
  switch (attack_type) {
    case "synFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, SYNFloodEnable, SYNFloodSYNThreshold, SYNFloodACKThreshold FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "tcpFrag":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, TCPFragmentEnable FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "land":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, LandAttackEnable FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "udpFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, UDPFloodEnable, UDPFloodThreshold, UDPFloodRate FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "udpFrag":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, UDPFragmentEnable FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "ipsec":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, IPSecIKEEnable, IPSecIKEThreshold FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "icmpFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, ICMPFloodEnable, ICMPFloodRate, ICMPFloodThreshold FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "dnsFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, DNSFloodEnable, DNSFloodThreshold FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "httpFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, HTTPFloodEnable FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    case "httpsFlood":
      sql = `SELECT DefenseProfileId, DefenseProfileType, DefenseProfileName, DefenseProfileDescription, DetectionTime, DefenseMode, HTTPSFloodEnable FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
      break;
    default:
      return Promise.reject(new Error("Invalid case provided"));
  }
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      } else {
        resolve(rows[0]);
      }
    });
  });
};

const getAllThresholdByActiveDefenseProfile = () => {
  const sql = `SELECT DetectionTime, DefenseMode, ICMPFloodRate, ICMPFloodThreshold, SYNFloodSYNThreshold, SYNFloodACKThreshold, UDPFloodThreshold, UDPFloodRate, DNSFloodThreshold, IPSecIKEThreshold FROM DefenseProfiles WHERE DefenseProfileStatus = 'Active'`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      } else {
        resolve(rows[0] || null);
      }
    });
  });
};

const updateDefenseProfile = (DefenseProfileId, profileUpdate) => {
  return new Promise((resolve, reject) => {
    if (!profileUpdate) {
      return reject(new Error("profileUpdate cannot be null or undefined"));
    }

    const sql = `UPDATE DefenseProfiles SET 
            DefenseProfileName = ?,
            DefenseProfileDescription = ?,
            DefenseProfileLastModified = ?,
            DefenseProfileUsingTime = ?,
            DefenseProfileType = ?,
            DefenseProfileStatus = ?,
            DetectionTime = ?,
            DefenseMode = ?,
            ICMPFloodEnable = ?,
            ICMPFloodThreshold = ?,
            ICMPFloodRate = ?,
            SYNFloodEnable = ?,
            SYNFloodSYNThreshold = ?,
            SYNFloodACKThreshold = ?,
            SYNFloodWhiteListTimeOut = ?,
            UDPFloodEnable = ?,
            UDPFloodThreshold = ?,
            UDPFloodRate = ?,
            DNSFloodEnable = ?,
            DNSFloodThreshold = ?,
            LandAttackEnable = ?,
            IPSecIKEEnable = ?,
            IPSecIKEThreshold = ?,
            TCPFragmentEnable = ?,
            UDPFragmentEnable = ?,
            HTTPFloodEnable = ?,
            HTTPSFloodEnable = ?
        WHERE DefenseProfileId = ?`;
    db.run(
      sql,
      [
        profileUpdate.DefenseProfileName,
        profileUpdate.DefenseProfileDescription,
        profileUpdate.DefenseProfileLastModified,
        profileUpdate.DefenseProfileUsingTime,
        profileUpdate.DefenseProfileType,
        profileUpdate.DefenseProfileStatus,
        profileUpdate.DetectionTime,
        profileUpdate.DefenseMode,
        profileUpdate.ICMPFloodEnable,
        profileUpdate.ICMPFloodThreshold,
        profileUpdate.ICMPFloodRate,
        profileUpdate.SYNFloodEnable,
        profileUpdate.SYNFloodSYNThreshold,
        profileUpdate.SYNFloodACKThreshold,
        profileUpdate.SYNFloodWhiteListTimeOut,
        profileUpdate.UDPFloodEnable,
        profileUpdate.UDPFloodThreshold,
        profileUpdate.UDPFloodRate,
        profileUpdate.DNSFloodEnable,
        profileUpdate.DNSFloodThreshold,
        profileUpdate.LandAttackEnable,
        profileUpdate.IPSecIKEEnable,
        profileUpdate.IPSecIKEThreshold,
        profileUpdate.TCPFragmentEnable,
        profileUpdate.UDPFragmentEnable,
        profileUpdate.HTTPFloodEnable,
        profileUpdate.HTTPSFloodEnable,
        DefenseProfileId,
      ],
      function (err) {
        if (err) {
          reject(err);
        } else {
          resolve(this.changes);
        }
      }
    );
  });
};

const insertDefenseProfile = (profile) => {
  return new Promise((resolve, reject) => {
    const sqlInsert = `INSERT INTO DefenseProfiles (${Object.keys(profile).join(
      ", "
    )}) VALUES (${Object.keys(profile)
      .map(() => "?")
      .join(", ")})`;
    const sqlGetId = `SELECT MAX(DefenseProfileId) AS id FROM DefenseProfiles`;

    db.serialize(() => {
      db.run("BEGIN TRANSACTION");

      db.run(sqlInsert, Object.values(profile), function (err) {
        if (err) {
          db.run("ROLLBACK");
          return reject(err);
        }

        db.get(sqlGetId, [], (err, row) => {
          if (err) {
            db.run("ROLLBACK");
            return reject(err);
          }

          db.run("COMMIT");
          resolve(row.id);
        });
      });
    });
  });
};

const deleteDefenseProfile = (DefenseProfileId) => {
  //DefenseProfileId is foreign key of interface table
  return new Promise((resolve, reject) => {
    const sql = `DELETE FROM DefenseProfiles WHERE DefenseProfileId = ?`;
    db.run(sql, [DefenseProfileId], function (err) {
      if (err) {
        reject(err);
      } else {
        resolve(this.changes);
      }
    });
  });
};

function updateDefenseProfileUsingTime(defenseProfileId, usageJson) {
  const sql = `UPDATE DefenseProfiles SET DefenseProfileUsingTime = ? WHERE DefenseProfileId = ?`;
  return new Promise((resolve, reject) => {
    db.run(sql, [usageJson, defenseProfileId], function (err) {
      if (err) reject(err);
      else resolve(this.changes);
    });
  });
}

module.exports = {
  getAllDefenseProfiles,
  getAllDefenseProfilesName,
  getAllDefenseProfilesbyUserId,
  getDefenseProfilesbyOffset,
  getDefenseProfilebyDefenseProfileId,
  getActiveDefenseProfile,
  getAttackTypeOfDefenseProfile,
  getAllThresholdByActiveDefenseProfile,
  updateDefenseProfile,
  insertDefenseProfile,
  deleteDefenseProfile,
  updateDefenseProfileUsingTime,
};
