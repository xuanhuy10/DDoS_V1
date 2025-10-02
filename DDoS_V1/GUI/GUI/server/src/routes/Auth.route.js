const express = require('express');
const router = express.Router();

const authCtrl = require('../controllers/Auth.controller');

// router.post('/register', authCtrl.register);

router.post('/login',   authCtrl.login);
router.post('/refresh', authCtrl.refreshToken);

module.exports = router;