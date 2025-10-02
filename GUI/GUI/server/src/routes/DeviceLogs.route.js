const express = require("express");
const router = express.Router();

const { ppAuthenticate } = require("../middleware/middleware/passsport");

const deviceLogsCtrl = require("../controllers/DeviceLogs.controller");

//#region database logs
// get
router.get("/", ppAuthenticate, deviceLogsCtrl.getAllDeviceLogs);
router.get(
  "/system/:logType",
  ppAuthenticate,
  deviceLogsCtrl.getDeviceLogByLogType
);

router.get(
  "/system/export/:logType/time-range",
  ppAuthenticate,
  deviceLogsCtrl.exportDeviceLogByTypeAndTimeRange
);
router.delete(
  "/system/:logType/time-range",
  ppAuthenticate,
  deviceLogsCtrl.deleteDeviceLogByTypeAndTimeRange
);

router.delete(
  "/system/:logType/:ids",
  ppAuthenticate,
  deviceLogsCtrl.deleteDeviceLogByLogTypeAndIds
);
router.get(
  "/system/export/:logType/:ids",
  ppAuthenticate,
  deviceLogsCtrl.exportDeviceLogByLogTypeAndIds
);

// router.get('/system/export/:logType', deviceLogsCtrl.exportLogSystembyLogType);
//#endregion

//#region traffic logs
// get
router.get("/traffic/:logType", deviceLogsCtrl.getTrafficLogByLogType);

// Thêm query format cho export

// delete
router.get(
  "/traffic/export/:logType/time-range",
  deviceLogsCtrl.exportLogTrafficByTypeAndTimeRange
);
router.delete(
  "/traffic/:logType/time-range",
  deviceLogsCtrl.deleteLogTrafficByTypeAndTimeRange
);
//#endregion

module.exports = router;
