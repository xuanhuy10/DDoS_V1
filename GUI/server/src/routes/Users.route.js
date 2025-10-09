const express = require("express");
const router = express.Router();
const { ppAuthenticate } = require("../middleware/middleware/passsport");

const userCtrl = require("../controllers/Users.controller");

router.get("/", ppAuthenticate, userCtrl.getAllUsers);
router.get("/:userId", ppAuthenticate, userCtrl.getUserById);
router.get("/session/me", ppAuthenticate, userCtrl.getLoggedInUser);
router.post("/", ppAuthenticate, userCtrl.insertUser);
router.patch("/:userId", ppAuthenticate, userCtrl.updateUser);
router.delete("/:userId", ppAuthenticate, userCtrl.deleteUser);

module.exports = router;
