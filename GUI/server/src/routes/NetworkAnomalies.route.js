const express = require('express');
const router = express.Router();

const { ppAuthenticate } = require('../middleware/middleware/passsport');

const networkAnomaliesCtrl = require('../controllers/NetworkAnomalies.controller');

// get
router.get('/anomalies', ppAuthenticate, networkAnomaliesCtrl.getAllNetworkAnomalies);

router.get('/analysis', ppAuthenticate, networkAnomaliesCtrl.getNetworkAnalysis);

module.exports = router;