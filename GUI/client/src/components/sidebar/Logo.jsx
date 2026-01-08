import React from "react";
import logo from "../../assets/logo.svg"; // Đường dẫn tới SVG
 
const Logo = () => {
   return (
      <div className="logo">
         <div className="logo-icon">
              <img src={logo} alt="Logo" style={{ width: 100, height: 60, marginTop:'40px',marginRight:'1px' }} />
         </div>
      </div>
   );
};
 
export default Logo;