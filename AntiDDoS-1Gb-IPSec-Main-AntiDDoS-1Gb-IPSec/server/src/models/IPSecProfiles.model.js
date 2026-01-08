const db = require("../helper/db.helper");

// create table IPSecProfiles
// (
//     IPSecProfileId      INTEGER
// primary key,
//     UserId              INTEGER
// references Users,
//     ProfileName         TEXT not null,
//     ProfileDescription  TEXT,
//     LocalGateway        TEXT,
//     RemoteGateway       TEXT,
//     IKEVersion          TEXT,
//     Mode                TEXT,
//     ESPAHProtocol       TEXT,
//     IKEReauthTime       INTEGER,
//     EncryptionAlgorithm TEXT,
//     HashAlgorithm       TEXT,
//     ReKeyTime           INTEGER,
//     Enable              BOOLEAN,
//     CreateTime          TEXT,
//     LastModified        TEXT,
//     SubnetLocalGateway  TEXT,
//     SubnetRemoteGateway TEXT,
//     ConnectionCount     INTEGER
// );


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
const insertIpSecProfile = (profile) => {
  return new Promise((resolve, reject) => {
    // Kiểm tra UserId
    const checkUserId = (userId) => {
      return new Promise((resolveInner, rejectInner) => {
        const sql = `SELECT UserId FROM Users WHERE UserId = ?`;
        db.get(sql, [userId], (err, row) => {
          if (err) return rejectInner(err);
          if (!row) return rejectInner(new Error(`UserId ${userId} does not exist in Users table`));
          resolveInner(row.UserId);
        });
      });
    };

    const prepareProfile = async () => {
      const profileCopy = { ...profile };
      if (!profileCopy.UserId) throw new Error("UserId is required");
      await checkUserId(profileCopy.UserId);
      profileCopy.CreateTime = profileCopy.CreateTime || new Date().toISOString();
      return profileCopy;
    };

    const isTrue = (v) => v === true || v === 1 || v === "1" || v === "true";

    prepareProfile()
        .then((profileData) => {
          const keys = Object.keys(profileData);
          const placeholders = keys.map(() => "?").join(", ");
          const sqlInsert = `INSERT INTO IPSecProfiles (${keys.join(", ")}) VALUES (${placeholders})`;

          db.serialize(() => {
            db.run("BEGIN TRANSACTION", (beginErr) => {
              if (beginErr) return reject(beginErr);

              db.run(sqlInsert, Object.values(profileData), function (err) {
                if (err) {
                  db.run("ROLLBACK");
                  return reject(err);
                }

                const insertedId = this.lastID; // <-- dùng this.lastID thay vì SELECT MAX(...)
                // Nếu enable = true (linh hoạt với nhiều kiểu giá trị), disable các profile khác cùng user trong transaction
                if (isTrue(profileData.Enable)) {
                  const disableSql = `UPDATE IPSecProfiles SET Enable = 0 WHERE IPSecProfileId != ? AND UserId = ? AND Enable = 1`;
                  db.run(disableSql, [insertedId, profileData.UserId], function (disableErr) {
                    if (disableErr) {
                      db.run("ROLLBACK");
                      return reject(disableErr);
                    }
                    db.run("COMMIT", (cErr) => {
                      if (cErr) {
                        db.run("ROLLBACK");
                        return reject(cErr);
                      }
                      resolve(insertedId);
                    });
                  });
                } else {
                  db.run("COMMIT", (cErr) => {
                    if (cErr) {
                      db.run("ROLLBACK");
                      return reject(cErr);
                    }
                    resolve(insertedId);
                  });
                }
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

    const updateData = {
      ...profileUpdate,
      LastModified: new Date().toISOString(),
    };

    const checkUserId = (userId) => {
      return new Promise((resolveInner, rejectInner) => {
        const sql = `SELECT UserId FROM Users WHERE UserId = ?`;
        db.get(sql, [userId], (err, row) => {
          if (err) return rejectInner(err);
          if (!row) return rejectInner(new Error(`UserId ${userId} does not exist in Users table`));
          resolveInner(true);
        });
      });
    };

    const prepare = async () => {
      if (updateData.UserId) {
        await checkUserId(updateData.UserId);
      }
      return updateData;
    };

    if (updateData.hasOwnProperty("IPSecProfileId")) {
      delete updateData.IPSecProfileId;
    }

    const fields = Object.keys(updateData);
    if (fields.length === 0) {
      return reject(new Error("No fields to update"));
    }

    prepare()
        .then((profileData) => {
          const setClause = Object.keys(profileData).map((key) => `${key} = ?`).join(", ");
          const sql = `UPDATE IPSecProfiles SET ${setClause} WHERE IPSecProfileId = ?`;
          const params = [...Object.values(profileData), IPSecProfileId];

          // Lấy UserId hiện tại từ DB (trước update)
          const fetchUserIdSql = `SELECT UserId FROM IPSecProfiles WHERE IPSecProfileId = ?`;
          db.get(fetchUserIdSql, [IPSecProfileId], (fetchErr, profileRow) => {
            if (fetchErr) return reject(fetchErr);
            if (!profileRow) return reject(new Error(`No profile found with IPSecProfileId: ${IPSecProfileId}`));

            // Nếu request muốn đổi UserId thì ưu tiên UserId mới; nếu không thì dùng từ DB
            const targetUserId = profileData.hasOwnProperty("UserId") ? profileData.UserId : profileRow.UserId;

            const isTrue = (v) => v === true || v === 1 || v === "1" || v === "true";

            db.serialize(() => {
              db.run("BEGIN TRANSACTION", (beginErr) => {
                if (beginErr) return reject(beginErr);

                let updateChanges = 0;
                db.run(sql, params, function (err) {
                  if (err) {
                    db.run("ROLLBACK");
                    return reject(err);
                  }
                  updateChanges = this.changes;
                  if (updateChanges === 0) {
                    db.run("ROLLBACK");
                    return reject(new Error(`No profile updated with IPSecProfileId: ${IPSecProfileId}`));
                  }

                  // Nếu sau update Enable = true --> disable các profile khác cùng UserId (dùng targetUserId)
                  if (isTrue(profileData.Enable)) {
                    const disableSql = `UPDATE IPSecProfiles SET Enable = 0 WHERE IPSecProfileId != ? AND UserId = ? AND Enable = 1`;
                    db.run(disableSql, [IPSecProfileId, targetUserId], function (disableErr) {
                      if (disableErr) {
                        db.run("ROLLBACK");
                        return reject(disableErr);
                      }
                      db.run("COMMIT", (cErr) => {
                        if (cErr) {
                          db.run("ROLLBACK");
                          return reject(cErr);
                        }
                        resolve(updateChanges);
                      });
                    });
                  } else {
                    db.run("COMMIT", (cErr) => {
                      if (cErr) {
                        db.run("ROLLBACK");
                        return reject(cErr);
                      }
                      resolve(updateChanges);
                    });
                  }
                });
              });
            });
          });
        })
        .catch(reject);
  });
};
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