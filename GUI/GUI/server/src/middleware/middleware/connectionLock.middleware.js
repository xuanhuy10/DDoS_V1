const { getConnectionLock } = require('../models/Defend.model');

const connectionLockMiddleware = async (req, res, next) => {
    try {
        const { ConnectionLock } = await getConnectionLock();
        if (ConnectionLock === 1) {
            req.session.error_message = 'The system is currently busy, please try again later';
            return res.redirect('/');
        }
        next();
    } catch (err) {
        console.error('Error checking connection lock: ', err);
        req.session.error_message = 'An error occurred, please try again later';
        res.redirect('/');
    }
}

module.exports = connectionLockMiddleware;