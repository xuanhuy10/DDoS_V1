const express = require('express');
const router = express.Router();
const multer = require('multer');
const upload = multer({ storage: multer.memoryStorage() });

const { ppAuthenticate } = require('../middleware/middleware/passsport');

const ipsecCtrl = require('../controllers/IPSec.controller');

/**
 * Pages: IP Security Profile
 */
router.get('/all/:offset?', ppAuthenticate, ipsecCtrl.getIpSecProfiles);
/**
 * Pages: Create IpSec Profile
 */
router.post('/combined', ppAuthenticate, upload.fields([{ name: 'ca-cert' }, { name: 'cert' }, { name: 'private-key' }]), ipsecCtrl.insertIpSecProfileAndCertificates);
/**
 * Pages: IP Security Profile
 */
router.patch('/:profileId', ppAuthenticate, ipsecCtrl.updateIpSecProfile);
/**
 * Pages: IP Security Profile
 */
router.delete('/:profileId', ppAuthenticate, ipsecCtrl.deleteIpSecProfile);


module.exports = router;