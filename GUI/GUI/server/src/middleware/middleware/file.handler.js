const multer = require('multer');
const fs = require('fs');
const path = require('path');


const storage = multer.diskStorage({
    destination: function (req, file, cb) {
        const uploadPath = path.join(__dirname, '../public/uploads/');

        // Check if the directory exists, if not, create it
        if (!fs.existsSync(uploadPath)) {
            fs.mkdirSync(uploadPath, { recursive: true });
        }

        cb(null, uploadPath);
    },
    filename: function (req, file, cb) {
        const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1E9);
        cb(null, file.originalname + '-' + uniqueSuffix + path.extname(file.originalname));
    }
});


const imageUpload = multer({ 
    storage: storage,
});

const uploadExcel = multer({ 
    storage: storage, 
}); 

const updateFileExcel = (username, file) => {
    const excelPath = path.join(__dirname, '..', 'public', 'uploads', username, 'Book.xlsx');
    const destinationDir = path.dirname(excelPath);

    if (!fs.existsSync(destinationDir)) {
        fs.mkdirSync(destinationDir, { recursive: true });
    }
    
    fs.rename(file.path, excelPath, (err) => {
        if (err) {
            console.error('Error saving excel:', err);
            throw new Error('Could not save the excel file');
        }
    });
};

const updateProfileImage = (username, file) => {
    // Path to save the avatar image
    const imagePath = path.join(__dirname, '..', 'public', 'uploads', username, 'avatar.png');
    const destinationDir = path.dirname(imagePath);

    if (!fs.existsSync(destinationDir)) {
        fs.mkdirSync(destinationDir, { recursive: true });
    }
    
    fs.rename(file.path, imagePath, (err) => {
        if (err) {
            console.error('Error saving image:', err);
            throw new Error('Could not save the profile image');
        }
    });
};

module.exports = {
    imageUpload,
    updateProfileImage,
    updateFileExcel,
    uploadExcel,
}