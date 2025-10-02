import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import Backend from "i18next-http-backend";
import LanguageDetector from "i18next-browser-languagedetector";

import en from './locales/en/translation.json';
import vi from './locales/vi/translation.json';

const resources = {
    en: {
        translation: en
    },
    vi: {
        translation: vi
    }
};

i18n
    .use(Backend)
    .use(LanguageDetector)
    .use(initReactI18next)
    .init({
        resources,
        lng: 'en',
        debug: true,
        interpolation: {
            escapeValue: false
        }
    });


export default i18n;
//