import React from "react";
import { Navigate } from "react-router-dom";
import { useAuth } from "@/hooks/useAuth";

const ProtectedRoute = ({ children, roleRequired }) => {
    const { user, loadAuth } =  useAuth();

    if (loadAuth) {
        return null;
    }
    
    if (!user) {
        return <Navigate to="/auth/login" />;
    }

    if (roleRequired && user.role !== roleRequired) {
        return <Navigate to="/unauthorized" />;
    }

    return children;
};

export default ProtectedRoute;
