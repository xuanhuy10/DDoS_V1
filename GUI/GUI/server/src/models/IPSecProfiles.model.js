const db = require("../helper/db.helper");
const {
    disableOtherProfiles,
} = require("../models/DeviceInterfaces.model");

// CREATE TABLE IPSecProfiles (
//     IPSecProfileId INTEGER PRIMARY KEY,
//     UserId INTEGER,
//     ProfileName TEXT NOT NULL,
//     ProfileDescription TEXT,
//     InterfaceId INTEGER,
//     LocalGateway TEXT,
//     RemoteGateway TEXT,
//     IKEVersion TEXT,
//     Mode TEXT,
//     ESPAHProtocol TEXT,
//     IKEReauthTime INTEGER,
//     EncryptionAlgorithm TEXT,
//     HashAlgorithm TEXT,
//     ReKeyTime INTEGER,
//     Enable BOOLEAN,
//     CreateTime TEXT,
//     LastModified TEXT,
//     UsingTime TEXT,
//     FOREIGN KEY (UserId) REFERENCES Users (UserId),
//     FOREIGN KEY (InterfaceId) REFERENCES DeviceInterfaces (InterfaceId)
// )

// Lấy tất cả IPSec Profiles
const getIpSecProfiles = () => {
  const sql = `SELECT * FROM IPSecProfiles`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

// Lấy IPSec Profiles theo offset (phân trang)
const getIpSecProfilesByOffset = (offset) => {
  const sql = `SELECT * FROM IPSecProfiles ORDER BY IPSecProfileId DESC LIMIT 10 OFFSET ?`;
  return new Promise((resolve, reject) => {
    db.all(sql, [offset], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

// Lấy IPSec Profile theo ID
const getIpSecProfileById = (IPSecProfileId) => {
  const sql = `SELECT * FROM IPSecProfiles WHERE IPSecProfileId = ?`;
  return new Promise((resolve, reject) => {
    db.get(sql, [IPSecProfileId], (err, row) => {
      if (err) {
        reject(err);
      }
      resolve(row);
    });
  });
};

// Lấy tất cả IPSec Profiles theo UserId
const getIpSecProfilesByUserId = (UserId) => {
  const sql = `SELECT * FROM IPSecProfiles WHERE UserId = ?`;
  return new Promise((resolve, reject) => {
    db.all(sql, [UserId], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

// Lấy tất cả IPSec Profiles đang active
const getActiveIpSecProfile = () => {
  const sql = `
        SELECT *
        FROM IPSecProfiles
                 LEFT OUTER JOIN Users ON IPSecProfiles.UserId = Users.UserId
        WHERE Enable = 1`;
  return new Promise((resolve, reject) => {
    db.all(sql, [], (err, rows) => {
      if (err) {
        reject(err);
      }
      resolve(rows);
    });
  });
};

// Lấy cấu hình IKE của IPSec Profile active
const getIpSecSettingsByActiveProfile = (settingType = null) => {
  let fields =
    "IKEVersion, Mode, ESPAHProtocol, IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, ReKeyTime";
  if (settingType) {
    // Logic lọc fields dựa trên settingType, ví dụ: if (settingType === 'ike') fields = 'IKEVersion, Mode, ...';
  }
  const sql = `SELECT ${fields} FROM IPSecProfiles WHERE Enable = 1`;
  return new Promise((resolve, reject) => {
    db.get(sql, [], (err, row) => {
      if (err) reject(err);
      resolve(row);
    });
  });
};

// Thêm mới IPSec Profile
const insertIpSecProfile = (profile) => {
  return new Promise((resolve, reject) => {
    // Kiểm tra UserId
    const checkUserId = (userId) => {
      return new Promise((resolveInner, rejectInner) => {
        const sql = `SELECT UserId FROM Users WHERE UserId = ?`;
        db.get(sql, [userId], (err, row) => {
          if (err) return rejectInner(err);
          if (!row)
            return rejectInner(
              new Error(`UserId ${userId} does not exist in Users table`)
            );
          resolveInner(row.UserId);
        });
      });
    };

    const prepareProfile = async () => {
      const profileCopy = { ...profile };

      // Kiểm tra UserId
      if (!profileCopy.UserId) {
        throw new Error("UserId is required");
      }
      await checkUserId(profileCopy.UserId);

      return profileCopy;
    };

    prepareProfile()
      .then((profileData) => {
        const sqlInsert = `INSERT INTO IPSecProfiles (${Object.keys(
          profileData
        ).join(", ")}) VALUES (${Object.keys(profileData)
          .map(() => "?")
          .join(", ")})`;
        const sqlGetId = `SELECT MAX(IPSecProfileId) AS id FROM IPSecProfiles`;

        db.serialize(() => {
          db.run("BEGIN TRANSACTION");
          db.run(sqlInsert, Object.values(profileData), function (err) {
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
      })
      .catch(reject);
  });
};

const updateIpSecProfile = (IPSecProfileId, profileUpdate) => {
  return new Promise((resolve, reject) => {
    if (!profileUpdate || Object.keys(profileUpdate).length === 0) {
      return reject(new Error("profileUpdate cannot be null, undefined, or empty"));
    }

    // Kiểm tra UserId nếu có trong profileUpdate (giống insert)
    const checkUserId = (userId) => {
      return new Promise((resolveInner, rejectInner) => {
        const sql = `SELECT UserId FROM Users WHERE UserId = ?`;
        db.get(sql, [userId], (err, row) => {
          if (err) return rejectInner(err);
          if (!row)
            return rejectInner(
                new Error(`UserId ${userId} does not exist in Users table`)
            );
          resolveInner(row.UserId);
        });
      });
    };

    const prepareProfile = async () => {
      const profileCopy = { ...profileUpdate };

      // Auto-set LastModified nếu không có
      if (!profileCopy.LastModified) {
        profileCopy.LastModified = new Date().toISOString();
      }

      // Kiểm tra UserId nếu có
      if (profileCopy.UserId) {
        await checkUserId(profileCopy.UserId);
      }

      // Loại trừ IPSecProfileId khỏi keys (không update ID)
      if (profileCopy.hasOwnProperty('IPSecProfileId')) {
        delete profileCopy.IPSecProfileId;
      }

      return profileCopy;
    };

    prepareProfile()
        .then((profileData) => {
          // Build dynamic SET clause giống insert
          const setClause = Object.keys(profileData)
              .map((key) => `${key} = ?`)
              .join(", ");
          const sql = `UPDATE IPSecProfiles SET ${setClause}, LastModified = COALESCE(?, LastModified) WHERE IPSecProfileId = ?`;

          // Params: values từ profileData + LastModified + IPSecProfileId
          const params = [
            ...Object.values(profileData),
            profileData.LastModified,
            IPSecProfileId,
          ];

          // Thực thi UPDATE trực tiếp (bỏ chain checkInterfaceId/disableOtherProfiles)
          db.run(sql, params, function (err) {
            if (err) {
              return reject(err);
            } else if (this.changes === 0) {
              return reject(new Error(`No profile found with IPSecProfileId: ${IPSecProfileId}`));
            }
            resolve(this.changes);
          });
        })
        .catch(reject);
  });
};

// Xóa IPSec Profile
const deleteIpSecProfile = (IPSecProfileId) => {
  return new Promise((resolve, reject) => {
    const sql = `DELETE FROM IPSecProfiles WHERE IPSecProfileId = ?`;
    db.run(sql, [IPSecProfileId], function (err) {
      if (err) {
        reject(err);
      } else {
        resolve(this.changes);
      }
    });
  });
};

module.exports = {
  getIpSecProfiles,
  getIpSecProfilesByOffset,
  getIpSecProfileById,
  getIpSecProfilesByUserId,
  getActiveIpSecProfile,
  getIpSecSettingsByActiveProfile,
  insertIpSecProfile,
  updateIpSecProfile,
  deleteIpSecProfile,
};
