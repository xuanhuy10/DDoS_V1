const dotenv = require('dotenv');

const passport = require('passport');
const passportJWT = require('passport-jwt');

const jwt = require('jsonwebtoken');

const ExtractJWT = passportJWT.ExtractJwt;
const Strategy = passportJWT.Strategy;

const jwtOptions = {
    jwtFromRequest: ExtractJWT.fromAuthHeaderAsBearerToken(),
    secretOrKey: process.env.SECRET_TOKEN,
};

const strategy = new Strategy(jwtOptions, (payload, next) => {
    const user = payload;
    next(null, user);
});

passport.use(strategy);

module.exports.ppAuthenticate = passport.authenticate('jwt', { session: false });