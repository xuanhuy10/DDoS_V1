const db = require("../helper/db.helper");

// create table DefenseProfiles
// (
//     DefenseProfileId           INTEGER
// primary key,
//     UserId                     INTEGER
// references Users,
//     DefenseProfileName         TEXT                        not null,
//     DefenseProfileDescription  TEXT                        not null,
//     DefenseProfileCreateTime   DATETIME                    not null,
//     DefenseProfileLastModified DATETIME                    not null,
//     DefenseProfileUsingTime    DATETIME,
//     DefenseProfileType         TEXT                        not null,
//     DefenseProfileStatus       TEXT                        not null,
//     DetectionTime              INTEGER default 1           not null,
//     DefenseMode                TEXT    default 'Aggregate' not null,
//     ICMPFloodEnable            BOOLEAN default 1           not null,
//     ICMPFloodThreshold         INTEGER default 1000        not null,
//     ICMPFloodRate              INTEGER default 1000        not null,
//     SYNFloodEnable             BOOLEAN default 1           not null,
//     SYNFloodSYNThreshold       INTEGER default 1000        not null,
//     SYNFloodACKThreshold       INTEGER default 1000        not null,
//     SYNFloodWhiteListTimeOut   INTEGER default 20          not null,
//     UDPFloodEnable             BOOLEAN default 1           not null,
//     UDPFloodThreshold          INTEGER default 1000        not null,
//     UDPFloodRate               INTEGER default 1000        not null,
//     DNSFloodEnable             BOOLEAN default 1           not null,
//     DNSFloodThreshold          INTEGER default 1000        not null,
//     LandAttackEnable           BOOLEAN default 1           not null,
//     IPSecIKEEnable             BOOLEAN default 1           not null,
//     IPSecIKEThreshold          INTEGER default 1000        not null,
//     TCPFragmentEnable          BOOLEAN default 1           not null,
//     UDPFragmentEnable          BOOLEAN default 1           not null,
//     HTTPFloodEnable            BOOLEAN default 1           not null,
//     HTTPSFloodEnable           BOOLEAN default 1           not null
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

const getAllDefenseProfilesByUserId = (UserId) => {
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
const getDefenseProfilesByOffset = (offset) => {
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

const getDefenseProfileByDefenseProfileId = (DefenseProfileId) => {
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
    const sqlInsert = `INSERT INTO DefenseProfiles (${Object.keys(profile).join(", ")}) VALUES (${Object.keys(profile)
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

module.exports = {
  getAllDefenseProfiles,
  getAllDefenseProfilesName,
  getAllDefenseProfilesByUserId,
  getDefenseProfilesByOffset,
  getDefenseProfileByDefenseProfileId,
  getActiveDefenseProfile,
  getAttackTypeOfDefenseProfile,
  getAllThresholdByActiveDefenseProfile,
  updateDefenseProfile,
  insertDefenseProfile,
  deleteDefenseProfile,
};
