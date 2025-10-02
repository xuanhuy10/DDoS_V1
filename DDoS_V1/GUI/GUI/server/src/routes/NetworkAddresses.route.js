const express = require('express');
const router = express.Router();

const { ppAuthenticate } = require('../middleware/middleware/passsport');

const networkAddressesCtrl = require('../controllers/NetworkAddresses.controller');

/**
 * Pages: Pages of Network Groups
 * Scope: all functions
 */

// GET
router.get('/common/white', ppAuthenticate,                 networkAddressesCtrl.getAllProtectedAddresses);
router.get('/common/black', ppAuthenticate,                 networkAddressesCtrl.getAllBlockedAddresses);
router.get('/vpn/white', ppAuthenticate,                    networkAddressesCtrl.getAllVPNAllowedAddresses);
router.get('/http/black', ppAuthenticate,                   networkAddressesCtrl.getAllHTTPBlockedAddresses);
// POST
router.post('/common/white', ppAuthenticate,                networkAddressesCtrl.insertProtectedAddresses);
router.post('/common/black', ppAuthenticate,                networkAddressesCtrl.insertBlockedAddresses);
router.post('/vpn/white', ppAuthenticate,                   networkAddressesCtrl.insertVPNAllowedAddresses);
router.post('/http/black', ppAuthenticate,                  networkAddressesCtrl.insertHTTPBlockedAddresses);
// POST BULK (FILE IMPORT)
router.post('/common/white/import', ppAuthenticate,         networkAddressesCtrl.importBulkProtectedAddresses);
router.post('/blocked/import', ppAuthenticate,              networkAddressesCtrl.importBulkBlockedAddresses);
router.post('/http/black/import', ppAuthenticate,           networkAddressesCtrl.importBulkHTTPBlockedAddresses);
router.post('/vpn/white/import', ppAuthenticate,            networkAddressesCtrl.importBulkVPNAllowedAddresses);
// POST BULK (FILE IMPORT DELETE)
router.post('/http/black/import-delete', ppAuthenticate,    networkAddressesCtrl.importDeleteBulkHTTPBlockedAddresses);
router.post('/vpn/white/import-delete', ppAuthenticate,     networkAddressesCtrl.importDeleteBulkVPNAllowedAddresses);
router.post('/blocked/import-delete', ppAuthenticate,       networkAddressesCtrl.importDeleteBulkBlockedAddresses);
router.post('/common/white/import-delete', ppAuthenticate,  networkAddressesCtrl.importDeleteBulkProtectedAddresses);
// DELETE
router.post('/bulk-delete', ppAuthenticate,                 networkAddressesCtrl.deleteNetworkAddressesByIds);
router.post('/bulk-delete-by-file', ppAuthenticate,         networkAddressesCtrl.deleteNetworkAddressesByAddressAndVersionList);
module.exports = router;