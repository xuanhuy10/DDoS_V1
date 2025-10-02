const express = require('express');
const router = express.Router();

const { ppAuthenticate } = require('../middleware/middleware/passsport');

const deviceInterfacesCtrl = require('../controllers/DeviceInterfaces.controller');

/**
 * Pages: General, Defense Profile, Dashboard, IP Security Profile
 */
router.get('/', ppAuthenticate, deviceInterfacesCtrl.getAllDeviceInterfaces);
/**
 * Pages: Port Mirroring
 */
router.get('/mirroring', ppAuthenticate, deviceInterfacesCtrl.getMirroringDeviceInterfaces);
/**
 * Pages: General
 */
router.patch('/:deviceInterfaceId', ppAuthenticate, deviceInterfacesCtrl.updateDeviceInterface);
/**
 * Pages: Port Mirroring
 */
router.post('/mirroring', ppAuthenticate, deviceInterfacesCtrl.createMirroringInterface);
/**
 * Pages: Port Mirroring
 */
router.patch('/mirroring/:MirrorInterfaceId', ppAuthenticate, deviceInterfacesCtrl.updateMirroringDeviceInterface);
/**
 * Pages: Port Mirroring
 */
router.delete('/mirroring/:MirrorInterfaceId', ppAuthenticate, deviceInterfacesCtrl.deleteMirroringInterface);



module.exports = router;