const express = require('express');
const router = express.Router();

const { ppAuthenticate } = require('../middleware/middleware/passsport');

const defenseProfilesCtrl = require('../controllers/DefenseProfiles.controller');

/**
 * Pages: General, Defense Profiles, Dashboard
 */
router.get('/all/:offset', ppAuthenticate,                  defenseProfilesCtrl.getDefenseProfiles);
/**
 * Pages: Dashboard
 */
router.get('/active', ppAuthenticate,                       defenseProfilesCtrl.getAllDefenseProfilesByActive);
/**
 * Pages: Subpages of Monitor
 */
router.get('/active/config/:attackType', ppAuthenticate,    defenseProfilesCtrl.getDefenseProfileAttackTypesConfig);
/**
 * Pages: Monitor, Subpages of Monitor
 */
router.get('/active/attacks/rate', ppAuthenticate,          defenseProfilesCtrl.getAllThresholdByActiveDefenseProfile);
/**
 * Pages: Create Defense Profile
 */
router.post('/', ppAuthenticate,                            defenseProfilesCtrl.insertDefenseProfile);
/**
 * Pages: General, Defense Profiles
 */
router.patch('/:profileId', ppAuthenticate,                 defenseProfilesCtrl.updateDefenseProfile);
/**
 * Pages: Defense Profiles
 */
router.post('/:profileId/apply', ppAuthenticate,            defenseProfilesCtrl.applyDefenseProfileToInterfaces);
/**
 * Pages: Defense Profiles
 */
router.delete('/:profileId', ppAuthenticate,                defenseProfilesCtrl.deleteDefenseProfile);

module.exports = router;