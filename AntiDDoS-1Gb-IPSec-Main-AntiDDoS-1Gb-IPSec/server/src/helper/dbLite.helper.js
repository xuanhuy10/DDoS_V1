var sqlite3 = require('sqlite3').verbose()
const config = require('../config')

const DBSOURCE = config.dblite.path

let db = new sqlite3.Database(DBSOURCE, (err) => {
    if (err) {
        // Cannot open database
        console.error(err.message)
        throw err
    } else {
        console.log('Database connected.')
        db.run('PRAGMA foreign_keys = ON;', (err) => {
            if (err) {
                console.error('Error enabling foreign keys:', err.message)
            } else {
                
            }
        });
    }
});

module.exports = db