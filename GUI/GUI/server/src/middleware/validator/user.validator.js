const { body } = require('express-validator');
const { getUserByUsername } = require('../models/User.model');

// Validator for changing password
const changePasswordValidator = [
    body('CHANGE_USER')
        .notEmpty().withMessage('Username is required')
        .isLength({ min: 5, max: 8 }).withMessage('Username is invalid')
        .bail()
        .custom(async (value) => {
            const user = await getUserByUsername(value);
            if (!user) {
                return Promise.reject('Username does not exist');
            }
        }),

    body('CHANGE_PASS')
        .notEmpty().withMessage('Password is required')
        .isLength({ min: 8, max: 10 }).withMessage('Password is invalid'),

    body('CONFIRM_PASS')
        .notEmpty().withMessage('Confirm Password is required')
        .custom((value, { req }) => {
            if (value !== req.body.CHANGE_PASS) {
                throw new Error('Password confirmation does not match password');
            }
            return true;
        })
];

// Validator for adding a new user
const addUserValidator = [
    body('ADD_USER')
        .notEmpty().withMessage('Username is required')
        .isLength({ min: 5, max: 8 }).withMessage('Username is invalid')
        .bail()
        .custom(async (value) => {
            const user = await getUserByUsername(value);
            if (user) {
                return Promise.reject('Username already exists');
            }
        }),

    body('ADD_EMAIL')
        .notEmpty().withMessage('Email is required')
        .isEmail().withMessage('Email is invalid'),

    body('ADD_PASS')
        .notEmpty().withMessage('Password is required')
        .isLength({ min: 8, max: 10 }).withMessage('Password is invalid'),

    body('CONFIRM_PASS')
        .notEmpty().withMessage('Confirm Password is required')
        .custom((value, { req }) => {
            if (value !== req.body.ADD_PASS) {
                throw new Error('Password confirmation does not match password');
            }
            return true;
        })
];

// Validator for deleting a user
const deleteUserValidator = [
    body('DELETE_USER')
        .notEmpty().withMessage('Username is required')
        .custom(async (value) => {
            const user = await getUserByUsername(value);
            if (!user) {
                return Promise.reject('User does not exist');
            } else if (user.Role === 'admin') {
                return Promise.reject('Cannot delete admin');
            }
        })
];

const updateInfoValidator = [
    body('CHANGE_USER')
        .notEmpty().withMessage('Username is required')
        .isLength({ min: 5, max: 8 }).withMessage('Username is invalid')
        .bail()
        .custom(async (value) => {
            const user = await getUserByUsername(value);
            if (!user) {
                return Promise.reject('Username does not exist');
            }
        }),

    body('CHANGE_EMAIL')
        .notEmpty().withMessage('Email is required')
        .isEmail().withMessage('Email is invalid'),

    body('CHANGE_NAME')
        .notEmpty().withMessage('Name is required'),

    //password is not always required and it may be empty so check if it is not empty then check if it is valid
    body('CHANGE_PASS')
        .if(body('CHANGE_PASS').notEmpty())
        .isLength({ min: 8, max: 10 }).withMessage('Password is invalid'),

    body('CONFIRM_PASS')
        //check if password is not empty
        .if(body('CHANGE_PASS').notEmpty())
        .notEmpty().withMessage('Confirm Password is required')
        .custom((value, { req }) => {
            if (value !== req.body.CHANGE_PASS) {
                throw new Error('Password confirmation does not match password');
            }
            return true;
        }),
];

// Exporting validators
const userValidator = {
    update: updateInfoValidator,
    create: addUserValidator,
    delete: deleteUserValidator
};

module.exports = userValidator;
