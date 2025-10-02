import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import { BrowserRouter } from 'react-router-dom'
import { I18nextProvider } from 'react-i18next';

import { AuthProvider } from './hooks/useAuth'

import './main.css'
import {App} from '@/app/app.jsx'

import './i18n'

createRoot(document.getElementById('root')).render(
  <AuthProvider>
    <StrictMode>
        <BrowserRouter>
          <App />
        </BrowserRouter>
    </StrictMode>
  </AuthProvider>
)
