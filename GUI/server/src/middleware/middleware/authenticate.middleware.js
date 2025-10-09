const jwt = require("jsonwebtoken");
const dotenv = require("dotenv");

dotenv.config();

const authenticate = (req, res, next) => {
    // Extract token from Authorization header
    const authHeader = req.headers["authorization"];
    const token = authHeader && authHeader.split(" ")[1]; // Extract "Bearer <token>"

    if (!token) {
        return res.status(401).json({ success: false, message: "You need to login first" });
    }

    // Verify the access token
    jwt.verify(token, process.env.SECRET_TOKEN, (err, user) => {
        if (err) {
            if (err.name === "TokenExpiredError") {
                return res.status(401).json({ success: false, message: "Access token expired" });
            }
            return res.status(403).json({ success: false, message: "Invalid access token" });
        }

        req.user = user; // Attach the user data to the request object
        next(); // Proceed to the next middleware or route handler
    });
};

// Middleware to enforce role-based access
const authorize = (requiredRole) => {
    return (req, res, next) => {
        const { user } = req;
        if (user && (user.Role === "admin" || user.Role === requiredRole)) {
            return next(); // User has the required role, proceed
        }

        return res.status(403).json({ success: false, message: "You do not have permission to access this resource" });
    };
};

module.exports = {
    authenticate,
    authorize,
};
