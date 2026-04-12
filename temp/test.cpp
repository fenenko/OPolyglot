#include <curl/curl.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/filefn.h> // Для wxRemoveFile
#include <atomic>
#include <fstream>
#include <sstream>
#include <string>

class SFDownloader {
private:
    std::atomic<bool> m_cancelFlag{false};
    std::string m_certData; // Зберігає сертифікат у пам'яті під час запиту
    wxString m_userAgent = "OPolyglot-Downloader/1.0";

    // 1. Callback для запису даних у файл
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        FILE *file = static_cast<FILE*>(userp);
        if (file) {
            return fwrite(contents, size, nmemb, file);
        }
        return 0;
    }

    // 2. Callback для прогресу та скасування
    static int ProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        SFDownloader* downloader = static_cast<SFDownloader*>(clientp);
        
        // Якщо прапорець встановлено в true, повертаємо 1, щоб перервати libcurl
        if (downloader && downloader->IsCancelled()) {
            return 1; 
        }

        // Тут можна викликати події wxWidgets для оновлення wxProgressDialog
        // (Наприклад: відправити wxThreadEvent з відсотком завантаження)
        
        return 0; // Продовжувати завантаження
    }

    // 3. Допоміжний метод: читання сертифіката
    bool LoadCertificate(const wxString& certFilePath, curl_blob& outBlob) {
        std::ifstream certFile(certFilePath.mb_str(), std::ios::binary);
        if (!certFile.is_open()) {
            wxLogError(wxT("Не вдалося відкрити файл сертифіката: %s"), certFilePath);
            return false;
        }

        std::ostringstream ss;
        ss << certFile.rdbuf();
        m_certData = ss.str(); // Зберігаємо як член класу, щоб дані жили під час завантаження

        outBlob.data = (void*)m_certData.c_str();
        outBlob.len = m_certData.size();
        outBlob.flags = CURL_BLOB_COPY;
        return true;
    }

public:
    SFDownloader() {}

    // Метод для виклику з GUI (натискання кнопки "Скасувати")
    void Cancel() {
        m_cancelFlag.store(true);
    }

    bool IsCancelled() const {
        return m_cancelFlag.load();
    }

    // Крок 1: Отримання розміру файлу
    curl_off_t GetFileSize(const wxString& url, const wxString& certFilePath) {
        CURL *curl = curl_easy_init();
        if (!curl) return -1;

        curl_off_t fileSize = -1;
        curl_blob certBlob;

        if (LoadCertificate(certFilePath, certBlob)) {
            curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &certBlob);
        } else {
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, (const char*)url.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Тільки заголовки
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, (const char*)m_userAgent.mb_str());

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
        } else {
            wxLogError(wxT("Помилка отримання розміру: %s"), wxString(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        return fileSize;
    }

    // Крок 2: Основне завантаження
    bool Download(const wxString& url, const wxString& outFilePath, const wxString& certFilePath) {
        // Скидаємо прапорець скасування перед новим завантаженням
        m_cancelFlag.store(false);

        CURL *curl = curl_easy_init();
        if (!curl) return false;

        bool success = false;
        curl_blob certBlob;

        if (!LoadCertificate(certFilePath, certBlob)) {
            curl_easy_cleanup(curl);
            return false;
        }

        // Відкриваємо файл для запису
        FILE *fp = fopen(outFilePath.mb_str(), "wb");
        if (!fp) {
            wxLogError(wxT("Не вдалося створити файл: %s"), outFilePath);
            curl_easy_cleanup(curl);
            return false;
        }

        // Налаштування URL та Сертифіката
        curl_easy_setopt(curl, CURLOPT_URL, (const char*)url.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &certBlob);
        
        // Налаштування SourceForge
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, (const char*)m_userAgent.mb_str());

        // Налаштування запису у файл
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        // Налаштування прогресу та скасування
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this); // Передаємо вказівник на поточний об'єкт (this)

        wxLogMessage(wxT("Починаємо завантаження: %s"), url);

        // Блокуючий виклик (качаємо файл)
        CURLcode res = curl_easy_perform(curl);

        fclose(fp);

        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code >= 200 && response_code < 300) {
                success = true;
                wxLogMessage(wxT("Файл успішно завантажено."));
            } else {
                wxLogError(wxT("Помилка HTTP сервера: %ld"), response_code);
            }
        } 
        else if (res == CURLE_ABORTED_BY_CALLBACK) {
            wxLogMessage(wxT("Завантаження скасовано користувачем."));
        } 
        else {
            wxLogError(wxT("Помилка libcurl: %s"), wxString(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);

        // Видаляємо сміття, якщо завантаження зірвалося чи було скасовано
        if (!success && wxFileExists(outFilePath)) {
            wxRemoveFile(outFilePath);
        }

        return success;
    }
};
