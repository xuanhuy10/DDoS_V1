import React, { createContext, useContext, useState, useEffect } from "react";
import { jwtDecode } from "jwt-decode";
import { tokenRefresh } from "@/utils/authApi";

const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
    const [user, setUser] = useState(null);
    const [loadAuth, setLoadAuth] = useState(true);

    useEffect(() => {
        const initializeAuth = async () => {
            setLoadAuth(true);
            const token = localStorage.getItem("access_token");
            
            if (token) {
                try {
                    const decoded = jwtDecode(token);
                    const now = Math.floor(Date.now() / 1000);
                    if (decoded.exp && decoded.exp < now) {
                        try {
                            const response = await tokenRefresh();
                            const { access_token } = response;

                            localStorage.setItem("access_token", access_token);
                            const decoded = jwtDecode(access_token);
                            setUser({ id: decoded.payload.Id, role: decoded.payload.Role });
                        } catch {
                            logout();
                        }
                    } else {
                        setUser({ id: decoded.payload.Id, role: decoded.payload.Role });
                    }
                } catch {
                    logout();
                }
            } else {
                logout();
            }
            setLoadAuth(false);
        };

        initializeAuth();
    }, []);

    const login = (data) => {
        localStorage.setItem("access_token", data.access_token);
        const decoded = jwtDecode(data.access_token);
        setUser({ id: decoded.payload.Id, role: decoded.payload.Role });
    };

    const logout = () => {
        localStorage.removeItem("access_token");
        setUser(null);
    };

    return (
        <AuthContext.Provider value={{ user, login, logout, loadAuth }}>
            {children}
        </AuthContext.Provider>
    );
};


export const useAuth = () => useContext(AuthContext);
