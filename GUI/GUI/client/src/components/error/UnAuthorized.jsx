// Unauthorized.js
import React from "react";
import { Result, Button } from "antd";
import { useNavigate } from "react-router-dom";

const Unauthorized = () => {
  const navigate = useNavigate();

  const handleBackHome = () => {
    navigate("/");
  };

  return (
    <Result
      status="403"
      title="401"
      subTitle="Sorry, you are not authorized to access this page."
      extra={
        <Button type="primary" 
        onClick={handleBackHome}
        >
          Back Home
        </Button>
      }
    />
  );
};

export default Unauthorized;
