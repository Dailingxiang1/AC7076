
#include "language_text.h"


#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
const char *lanuage_display_array[] = {"简体中文", "English", "日本語", "한국어", "Deutsch", "Français", "Español", "Ελληνικά", \
                                       "Русский язык", "Português", "ภาษาไทย", "Italiano", "العربية", "Türkçe", "فارسی", "Polskie", \
                                       "Bahasa Indonesia", "हिन्दी", "Nederlands", "Tiếng Việt", "היברו", "Bahasa Melayu", "ဗာရန်", "繁體中文"
                                      };


const char *language_switch_array[] = {"语言切换", "Language Switch", "言語切り替え", "언어 스위치", "Sprach umschaltung", "Changement de langue", \
                                       "Cambio de idioma", "Αλλαγή γλώσσας", "Переключение языка", "Alteração de idioma", "สลับภาษา", "Interruttore di lingua", \
                                       "تبديل اللغة", "Dil değiştirme", "سودهی زبان", "Przełącznik języka", "Beralih bahasa", "भाषा स्विच", \
                                       "Taalschakelaar", "Chuyển đổi ngôn ngữ", "מתג שפה", "Tukar bahasa", "ဘာသာစကား ပြောဆိုမှု", "語言切換"
                                      };

const char *noise_reduction_title[] = {"降噪", "Noise Reduction", "ノイズリダクション", "소음 감소", "Rausch unterdrückung", "Réduction du bruit", "Reducción de ruido", "Μείωση θορύβου", \
                                       "Шумоподавление", "Redução de ruído", "ลดเสียงรบกวน", "Riduzione del rumore", "تقليل الضوضاء", "Gürültü azaltma", "کاهش صدا", "Redukcja szumów", \
                                       "Pengurangan kebisingan", "शोर कम", "Geluidsreductie", "Giảm tiếng ồn", "הפחתת רעש", "Pengurangan bunyi", "ဥပမာ ၊", "降噪"
                                      };

const char *background_title[] = {"背景壁纸", "Background Wallpaper", "背景壁紙", "배경 벽지", "Hintergrund Tapete", "Fond d'écran", "Fondo Fondos de pantalla", "Ταπετσαρία φόντου", \
                                  "Фон Обои", "Fundo Papel de parede", "พื้นหลังวอลล์เปเปอร์", "Sfondo carta da parati", "خلفيات خلفية", "Arka plan duvar kağıdı", "کاغذ دیواری زمینه", "Tapeta w tle", \
                                  "Latar Belakang Wallpaper", "पृष्ठभूमि वॉलपेपर", "Achtergrond behang", "Hình nền nền", "טפט רקע", "Kertas dinding latar belakang", "နောက်ခံဖောက်", "背景壁紙"
                                 };

const char *brightness_title[] = {"屏幕亮度", "Brightness", "画面の明るさ", "화면 밝기", "Bildschirm helligkeit", "Luminosité de l'écran", "Brillo de la pantalla", "Φωτεινότητα οθόνης", \
                                  "Яркость экрана", "Brilho da tela", "ความสว่างหน้าจอ", "Luminosità dello schermo", "سطوع الشاشة", "Ekran parlaklığı", "روشنایی صفحه", "Jasność ekranu", \
                                  "Kecerahan layar", "स्क्रीन चमक", "Schermhelderheid", "Độ sáng màn hình", "בהירות מסך", "Kecerahan skrin", "မျက်နှာဖျက်ခြင်း", "屏幕亮度"
                                 };

const char *income_title[] = {"来电", "Call", "着信", "전화", "Anruf", "Appel entrant", "Llamada entrante", "Εισερχόμενη κλήση", \
                              "Входящие звонки", "Chamada", "สายเรียกเข้า", "Chiama", "اتصل", "Gelen çağrı", "تماس", "Zadzwoń", \
                              "Panggilan masuk", "कॉल करना", "Bel", "Cuộc gọi đến", "שיחה", "Panggilan", "ခေါ်ဆိုမှု", "來電"
                             };

const char *eq_title[] = {"均衡器", "Equalizer", "イコライザ", "이퀄라이저", "Der Equalizer", "Égaliseur", "Ecualizador", "ισοσταθμιστής", \
                          "Эквалайзер", "Equalizador", "ควอไลเซอร์", "Equalizzatore", "المعادل", "Ekolayzır", "برابری دهنده", "Korektor", \
                          "Alat penyeimbang", "समतुल्यकारक", "Equalizer", "Bộ cân bằng", "אקולייזר", "Penyamaan", "ဟိ", "均衡器"
                         };

const char *eq_mode_list[][5] = {{"标准", "摇滚", "流行", "经典", "爵士"}, {"Standard", "rock", "pop", "classic", "jazz"}, \
    {"標準", "ロック", "流行", "クラシック", "ジャズ"}, {"표준", "로큰롤", "인기", "클래식", "재즈"}, \
    {"Standard", "Rock", "Pop", "Klassisch", "Jazz"}, {"Normes", "Rock", "Populaire", "Classique", "Jazz"}, \
    {"Estándar", "Roca", "Popular", "Clásico", "Jazz"}, {"πρότυπο", "Βράχος", "δημοφιλής", "κλασική", "Τζαζ"}, \
    {"Стандарты", "Рок-н-ролл", "Популярные", "Классика", "Джаз"}, {"Padrão", "Rock & Roll", "Popular", "Clássico", "Jazz"}, \
    {"มาตรฐาน", "ร็อค", "ยอดนิยม", "คลาสสิก", "แจ๊ส"}, {"Standard", "Rock and roll", "Popolare", "Classico", "Jazz"}, \
    {"المعايير", "موسيقى الروك أند رول", "شعبية", "الكلاسيكية", "سيدي"}, {"Standart", "Rock and Roll", "Popüler", "Klasik", "Caz."}, \
    {"استاندارد", "راک", "محبوب", "کلاسی", "جاز"}, {"Standardowy", "Skała", "Popularny", "Klasyczny", "Jazz"}, \
    {"Standar", "Batu", "Populer", "Klasik", "Jazz"}, {"मानक", "रॉक", "लोकप्रिय", "क्लासिक", "जैज़"}, \
    {"Standaard", "Rots", "Populair", "Klassiek", "Jazz"}, {"Tiêu chuẩn", "Đá", "Phổ biến", "Kinh điển", "Nhạc jazz"}, \
    {"סטנדרט", "רוק", "פופולרי", "קלאסי", "ג 'אז"}, {"Standard", "Batu", "Popular", "Klasik", "Jazz"}, \
    {"စံနှံး", "က် စ်", "ဥပမာ ၊", "ဥပမာ ၊", "ဂျာ"}, {"標準", "搖滾", "流行", "經典", "爵士"}
};

const char *find_earphone_title[] = {"查找耳机", "Find headphones", "イヤホンを探す", "헤드폰 찾기", "Finden Sie Kopfhörer", "Trouver des écouteurs", "Encuentra auriculares", "Βρείτε ακουστικά", \
                                     "Найти наушники", "Encontrar fones de ouvido", "ค้นหาหูฟัง", "Trova le cuffie", "البحث عن سماعات الرأس", "Kulaklık bul", "پیدا کردن تلفن های سرباز", "Znajdź słuchawki", \
                                     "Cari Headphone", "हेडफ़ोन खोजें", "Koptelefoons vinden", "Tìm tai nghe", "למצוא אוזניות", "Cari fon kepala", "Hedphones ရှာပါ", "查找耳機"
                                    };

const char *take_off_earphone_title[] = {"摘下耳机", "Take Off Earphone", "イヤホンを外す", "헤드폰 벗기", "Nehmen Sie die Kopfhörer ab", "Enlever les écouteurs", "Quítate los auriculares", "Βγάλτε τα ακουστικά.", \
                                         "Снимите наушники", "Tire os fones de ouvido", "ถอดหูฟัง", "Togliti le cuffie", "خلع سماعات الرأس", "Kulaklığı çıkar.", "صدف ها رو در بيار", "Zdejmij słuchawki", \
                                         "Melepas headphone", "हेडफ़ोन को हटा दें", "Doe de koptelefoon af", "Tháo tai nghe", "תוריד את האוזניות", "Keluarkan fon kepala", "ဒုတိယ ၊", "摘下耳機"
                                        };

const char *light_title[] = {"灯光", "Lights", "ライト", "조명", "Lichter", "Lumières", "Luces", "Φώτια", \
                             "Свет", "Luzes", "ไฟ", "Luci", "الأضواء", "Işıklar", "نورها", "Światła", \
                             "Lampu", "रोशनी", "Verlichting", "Đèn chiếu sáng", "אורות", "Lampu", "စာရင်းများ", "燈光"
                            };

const char *music_mode_label[][2] = {{"本地音乐", "手机音乐"}, {"SD Music", "Mobile phone music"}, {"ローカル音楽", "携帯電話の音楽"}, \
    {"지역 음악", "휴대 전화 음악"}, {"Lokale Musik", "Mobile Musik"}, {"Musique locale", "Musique mobile"}, \
    {"Música local", "Música móvil"}, {"Τοπική μουσική", "Κινητό τηλέφωνο μουσική"}, {"Местная музыка", "Мобильная музыка"}, \
    {"Música local", "Música para celular"}, {"เพลงท้องถิ่น", "เพลงมือถือ"}, {"Musica locale", "Musica del telefono cellulare"}, \
    {"الموسيقى المحلية", "موسيقى الهاتف المحمول"}, {"Yerel müzik", "Mobil müzik"}, {"موسیقی محلی", "موزیک تلفن همراهی"}, \
    {"Lokalna muzyka", "Muzyka z telefonu komórkowego"}, {"Musik Lokal", "Musik ponsel"}, {"स्थानीय संगीत", "मोबाइल फोन संगीत"}, \
    {"Lokale muziek", "Mobiele telefoon muziek"}, {"Âm nhạc địa phương", "Nhạc điện thoại di động"}, {"מוזיקה מקומית", "מוסיקה טלפון נייד"}, \
    {"Muzik Tempatan", "Muzik mudah alih"}, {"ဒေသတွင်းဂီတယ်", "ဆော့ဝဲ ဖီဒီဂီတ"}, {"本地音樂", "手機音樂"}
};

const char *pc_mode_title[] = {"文件传输", "File Transmission", "ファイル転送", "파일 전송", "Datei übertragung", "Transfert de fichiers", "Transferencia de archivos", "μεταφορά αρχείων", \
                               "Передача файлов", "Transferência de arquivos", "การถ่ายโอนไฟล์", "Trasferimento file", "نقل الملفات", "Dosya aktarımı", "انتقال پرونده", "Transfer plików", \
                               "Transfer file", "फाइल ट्रांसफर", "Bestandsoverdracht", "Chuyển tập tin", "העברת קבצים", "Pemindahan fail", "ဖိုင်လှမ်းပို့", "文件傳輸"
                              };

const char *usb_plug_in_title[] = {"请插入usb", "Please plug in the usb", "Usbを挿入してください", "Usb를 삽입하십시오", "Bitte fügen Sie den USB ein", "Veuillez insérer usb", "Por favor inserte el usb", "Παρακαλώ εισάγετε USB", \
                                   "Пожалуйста, вставьте usb", "Por favor, insira o usb", "กรุณาใส่ usb", "Si prega di inserire USB", "يرجى إدراج usb", "Lütfen usb takın", "لطفا درج usb", "Proszę włożyć usb", \
                                   "Silahkan masukkan usb", "कृपया यूएसबी सम्मिलित करें", "Gelieve te voegen usb", "Xin vui lòng chèn usb", "אנא הכנס usb", "Sila masukkan usb", "Steb ထည့်သွင်းပါ", "請插入usb"
                                  };


const char *time_setting[] = {"设置", "setting", "設定", "설정", "Einstellungen", "Paramètres", "Configuración", "ρύθμιση", \
                              "Настройка", "Configurações", "การตั้งค่า", "Setup", "الإعدادات", "Ayarlar", "برپایی", "Konfiguracja", \
                              "Pengaturan", "सेटअप", "Opstelling", "Cài đặt", "התקנה", "Tetapan", "နောက်ပိုင်းမှတ်", "設置"
                             };


const char *lock_title[] = {"锁屏壁纸", "Screen Lock", "ロック壁紙", "잠금 화면 벽지", "Lock Screen Wallpaper", "Écran de verrouillage Fond d'écran", \
                            "Fondos de pantalla de bloqueo", "Ταπετσαρία κλειδώματος οθόνης", "Блокировка экрана обои", "Tela de bloqueio Papel de parede", "วอลล์เปเปอร์ล็อคหน้าจอ", "Carta da parati della schermata di blocco", \
                            "خلفيات شاشة القفل", "Kilit ekranı duvar kağıdı", "قفل کردن کاغذ دیواری پردهName", "Ekran blokady tapety", "Wallpaper layar kunci", "लॉक स्क्रीन वॉलपेपर", \
                            "Achtergrond van vergrendelingsscherm", "Hình nền màn hình khóa", "נעילת טפט מסך", "Kertas dinding skrin kunci", "မျက်နှာဖုံးကို ဖန်တီးပါ", "鎖屏壁紙"
                           };

const char *music_title[] = {"音乐", "Music", "音楽", "음악", "Musik", "Musique", "Música", "Μουσική", "Музыка", "Música", "เพลง", "Musica", \
                             "الموسيقى", "Müzik", "موزیک", "Muzyka", "Musik", "संगीत", "Muziek", "Âm nhạc", "מוזיקה", "Muzik", "ဂီတ", "音樂"
                            };

const char *Volume_title[] = {"音量", "Volume", "ボリューム", "볼륨", "Lautstärke", "Volume", "Volumen", "Τόμος", "Объем", "Volume", "ระดับเสียง", "Volume", \
                              "مستوى الصوت", "Ses seviyesi", "حجم صدا", "Głośność", "Volume", "मात्रा", "Volume", "Âm lượng", "נפח", "Kelantangan", "အသံဗွက်မှု", "音量"
                             };

const char *equ_title[] = {"均衡", "Equilibrium", "均衡", "평형", "Ausgleich", "Équilibre", "Equilibrio", "Ισορροπή", "Равновесие", "Equilíbrio", "สมดุล", "Equilibrio", \
                           "التوازن", "Dengeli", "محکم", "Równowaga", "Keseimbangan", "संतुलन", "Evenwicht", "Cân bằng", "שיווי משקל", "Seimbang", "ဥပမာ ၊", "均衡"
                          };

const char *Alarm_title[] = {"闹钟", "Alarm Clock", "目覚まし時計", "알람 시계", "Wecker", "Réveil", "Despertador", "Ρολόι συναγερμού", "Будильник", "Despertador", "นาฬิกาปลุก", "Sveglia", \
                             "المنبه", "Çalar saat", "ساعت هشدارName", "Budzik", "Jam alarm", "अलार्म घड़ी", "Wekker", "Đồng hồ báo thức", "שעון מעורר", "Jam penggera", "မှတ်တမ်း", "鬧鐘"
                            };

const char *wallpaper_title[] = {"壁纸", "Wallpaper", "壁紙", "벽지", "Tapete", "Fond d'écran", "Fondos de pantalla", "Ταπετσαρία", "Обои", "Papel de parede", "วอลเปเปอร์", "Carta da parati", \
                                 "خلفيات", "Duvar kağıtları", "کاغذ دیواری", "Tapeta", "Wallpaper", "वॉलपेपर", "Behang", "Hình nền", "טפט", "Kertas dinding", "ကွန်ဖတ်က်", "壁紙"
                                };

const char *Weather_title[] = {"天气", "Weather", "天気", "날씨", "Wetter", "Météo", "El tiempo", "Καιρός", "Погода", "Tempo", "สภาพอากาศ", "Meteo", \
                               "الطقس", "Hava durumu", "هوا", "Pogoda", "Cuaca", "मौसम", "Weer", "Thời tiết", "מזג אוויר", "Cuaca", "တည်းခိုး", "天氣"
                              };

const char *wooden_fish_title[] = {"木鱼", "wooden fish", "木魚", "나무 물고기", "Holz fisch", "Poisson en bois", "Pez de madera", "Ξύλινα ψάρια", "Деревянная рыба", "Peixe de madeira", "ปลาไม้", "Pesce di legno", \
                                   "سمك الخشب", "Tahta balık", "ماهی چوب", "Drewniane ryby", "Ikan kayu", "लकड़ी की मछली", "Houten vis", "Cá gỗ", "דג עץ", "Ikan kayu", "ဥပမာ ၊", "木魚"
                                  };

const char *clock_title[] = {"时钟", "Clock", "時計", "시계", "Uhr", "Horloge", "Reloj", "Ρολόι", "Часы", "Relógio", "นาฬิกา", "Orologio", \
                             "على مدار الساعة", "Saat", "ساعت", "Zegar", "Jam", "घड़ी", "Klok", "Đồng hồ", "שעון", "Jam", "အဒေတား", "時鐘"
                            };

const char *Tiktok_title[] = {"抖音", "Tiktok", "ディザトーン", "티톡", "Douyin", "Take Sound", "Douyin", "Tiktok", "Встряхивание", "Tremendo", "ทิกส์", "Tiktok", \
                              "اهتزاز الصوت", "Titreme sesi", "تیروک", "Tiktok", "Tik Tok", "टिकटोक", "Tiktok", "Tik Tok", "טיקטוק", "Douyin", "Tidesktop မှတ်တမ်း", "抖音"
                             };

const char *Take_pictures_title[] = {"拍照", "Take pictures", "写真を撮る", "사진 찍기", "Fotografieren", "Prendre des photos", "Toma una foto", "Λήψη εικόνων", "Сфотографируйте", "Tire fotos", "ถ่ายรูป", "Scatta foto", \
                                     "التقاط الصور", "Fotoğraf çekmek", "گرفتن عکس", "Rób zdjęcia", "Mengambil foto", "तस्वीरें लें", "Maak foto's", "Chụp ảnh", "לצלם תמונות", "Ambil gambar", "ပုံများကို ယူပါ", "拍照"
                                    };

const char *Take_About_title[] = {"关于", "About", "について", "에 관하여", "Über", "À propos de", "Acerca de", "Σχετικά με το θέμα.", "О.", "Sobre", "เกี่ยวกับ", "A proposito", \
                                  "حول", "Hakkında", "درباره", "O", "Tentang", "के बारे में", "Over", "Về", "אודות", "Mengenai", "အကြောင်း", "關於"
                                 };

const char *app_title[] = {"app", "app", "アプリ", "앱", "App", "App", "Aplicación", "Εφαρμογή", "Приложение", "Aplicativo", "แอปพลิเคชั่น", "App", \
                           "التطبيق", "App", "برنامه", "Aplikacja", "Aplikasi", "एप", "App", "Ứng dụng", "אפליקציה", "App", "အကက်ပာ", "應用程序"
                          };

const char *ble_title[] = {"ble", "ble", "ブル", "블", "Ble", "Ble", "Por ble", "Μπελά", "Ble", "Bla", "BLE BLE", "Ble", \
                           "بلي", "Ble", "بلو", "Ble", "Ble", "बल", "Ble", "BLE", "בל", "Ble", "Blee", "Ble"
                          };

const char *Touch_title[] = {"触摸设置", "Touch Settings", "タッチ設定", "터치 설정", "Touch-Einstellungen", "Paramètres tactiles", "Configuración táctil", "Ρυθμίσεις αφής", "Настройки касания", "Configurações de toque", "สัมผัสการตั้งค่า", "Impostazioni touch", \
                             "إعدادات اللمس", "Dokunma Ayarları", "تنظیمات لمس", "Ustawienia dotykowe", "Pengaturan sentuh", "सेटिंग्स स्पर्श करें", "Aanraakinstellingen", "Cài đặt cảm ứng", "הגדרות מגע", "Tetapan sentuh", "ချက်ပြင်ဆင်မှုများ", "觸摸設置"
                            };

const char *reset_title[] = {"重启", "Restart", "再起動", "다시 시작", "Neustart", "Redémarrer", "Reiniciar", "Επανεκκίνηση", "Перезагрузка", "Reinicialização", "รีสตาร์ท", "Riavvia", "إعادة التشغيل", \
                             "Yeniden başlat", "آغازین", "Ponowne uruchomienie", "Mulai ulang", "पुनः प्रारंभ करें", "Opnieuw opstarten", "Khởi động lại", "הפעלה מחדש", "Mulakan semula", "ပြန်စစနစ်", "重啟"
                            };

const char *record_title[] = {"恢复出厂设置", "Restore factory settings", "工場出荷時の設定に戻す", "공장 설정 복원", "Werks einstellungen wiederherstellen", "Restaurer les paramètres d'usine", "Restablecimiento de la configuración de fábrica", "Επαναφορά ρυθμίσεων εργοστασίου", \
                              "Восстановление заводских настроек", "Restaurar as configurações de fábrica", "คืนค่าการตั้งค่าจากโรงงาน", "Ripristina le impostazioni di fabbrica", "استعادة إعدادات المصنع", "Fabrika ayarlarına sıfırlama", "بازگرداندن تنظیمات کارخانه", "Przywróć ustawienia fabryczne", \
                              "Kembalikan pengaturan pabrik", "कारखाना सेटिंग्स पुनर्स्थापित करें", "Fabrieksinstellingen herstellen", "Khôi phục cài đặt gốc", "שחזור הגדרות המפעל", "Pulihkan tetapan kilang", "ပြောင်းပြင်ထားချက်ကို ပြန်လည်ပြန်ထားပါ", "恢復出廠設置"
                             };

const char *shutdown_title[] = {"关机", "Shutdown", "電源を切る", "셧다운", "Herunter fahren", "Arrêt", "Apagado", "Κλείσιμο", "Выключить", "Desligue", "ปิดเครื่อง", "Arresto", \
                                "أغلق", "Kapatma", "خاموش", "Wyłączenie", "Matikan", "बंद करना", "Afsluiten", "Tắt máy", "כיבוי", "Tutup", "ပိတ်ပါ", "關機"
                               };

const char *save_title[] = {"保存", "Save", "保存", "저장", "Speichern", "Enregistrer", "Guardar", "Αποθήκευση", "Сохранить", "Salvar", "บันทึก", "Salva", \
                            "حفظ", "Kaydet", "ذخیره", "Zapisz", "Simpan", "सहेजना", "Bespaar", "Lưu", "חסכו", "Simpan", "သိမ်းဆည်း", "保存"
                           };

const char *return_title[] = {"返回", "Return", "戻る", "반환", "Zurück", "Retour", "Volver", "Επιστροφή", "Возвращение", "Voltar", "กลับ", "Reso", \
                              "العودة", "Geri dön", "برگشت", "Powrót", "Kembali", "लौटना", "Terugkeer", "Trở về", "חזרה", "Kembali", "ပြန်လာသည်", "返回"
                             };

const char *voice_title[] = {"声音设置", "Sound Settings", "サウンド設定", "사운드 설정", "Sound-Einstellungen", "Paramètres sonores", "Configuración de sonido", "Ρυθμίσεις ήχου", "Настройки звука", "Configurações de som", "การตั้งค่าเสียง", "Impostazioni del suono", \
                             "إعدادات الصوت", "Ses Ayarları", "تنظیمات صدا", "Ustawienia dźwięku", "Pengaturan suara", "ध्वनि सेटिंग", "Geluidsinstellingen", "Cài đặt âm thanh", "הגדרות קול", "Tetapan Bunyi", "အသံ ပြုပြင်မှုမျာ", "聲音設置"
                            };
const char *style_text_title[] = {"风格", "Style", "スタイル", "스타일", "Stil", "Style", "Estilo", "ΣτυλName", "Стиль", "Estilo", "สไตล์", "Stile", \
                                  "أسلوب", "Stil", "سبک", "Styl", "Gaya", "शैली", "Stijl", "Phong cách", "סגנון", "Gaya", "စတ်", "風格"
                                 };

const char *transfer_text_title[] = {"usb传输", "usb transmission", "Usb転送", "Usb 전송", "USB-Übertragung", "Transmission usb", "Transmisión usb", "Μετάδοση usb", "Usb передачи", "Transmissão usb", "การถ่ายโอน usb", "Trasmissione usb", \
                                     "نقل usb", "Usb iletimi", "انتقال usb", "Transmisja USB", "Transmisi usb", "यूएसबी ट्रांसमिशन", "Usb transmissie", "Truyền usb", "שידור usb", "Penghantaran usb", "Usb ပြုပြင်ပါ", "Usb傳輸"
                                    };

const char *transfer_phone_title[] = {"电话", "Telephone", "電話", "전화", "Telefon", "Tééphonel", "Teléfono", "Τηλέφωνο", "Телефон", "Telefone", "โทรศัพท์", "Telefono", \
                                      "الهاتف", "Telefon", "تلفنی", "Telefon", "Telepon", "टेलीफोन", "Telefoon", "Điện thoại", "טלפון", "Telefon", "ဖုန်း ဖက်", "電話"
                                     };

const char *call_record_title[] = {"暂无通话记录", "No call record", "通話記録はありません", "통화 기록 없음", "Keine Anruf aufzeichnung", "Pas d'enregistrement des appels", "Sin registro de llamadas", "Κανένα αρχείο κλήσης", \
                                   "Нет записи звонков", "Nenhum registro de chamadas", "ไม่มีประวัติการโทร", "Nessun record di chiamata", "لا يوجد سجل مكالمات", "Arama kaydı yok", "هیچ رکورد تماسی نیست", "Brak zapisów połączeń", \
                                   "Tidak ada catatan panggilan", "कॉल रिकॉर्ड नहीं", "Geen oproeprecord", "Không có bản ghi cuộc gọi", "אין שיא שיחות", "Tiada rekod panggilan", "ခေါ်ဆိုမှု အရေးပါ", "暫無通話記錄"
                                  };

const char *call_dial_pad_title[] = {"拨号键盘", "Dial keypad", "ダイヤルキーボード", "다이얼 키패드", "Wähl tastatur", "Clavier de numérotation", "Teclado de marcación", "Πληκτρολόγιο κλήσης", \
                                     "Клавиатура набора номера", "Teclado de discagem", "แป้นพิมพ์โทรออก", "Quadrante per tastiera", "لوحة مفاتيح الاتصال الهاتفي", "Çevirici klavye", "صفحه کلید شماره گیر", "Dial klawiatura", \
                                     "Keyboard panggilan", "डायल कीपैड", "Toetsenbord wijzerplaat", "Bàn phím quay số", "מקלדת חיוג", "Papan kekunci dail", "ကီးဒီစာရာ", "撥號鍵盤"
                                    };

const char *call_recording_title[] = {"通话记录", "Call recording", "通話記録", "통화 녹음", "Anruf protokoll", "Enregistrement des appels", "Grabación de llamadas", "Καταγραφή κλήσεων", \
                                      "Запись звонков", "Registro de chamadas", "บันทึกการโทร", "Registrazione delle chiamate", "سجل المكالمات", "Arama kaydı", "ضبط تماس", "Nagrywanie rozmów", \
                                      "Rekaman panggilan", "कॉल रिकॉर्डिंग", "Oproepopname", "Ghi âm cuộc gọi", "הקלטת שיחות", "Rekod panggilan", "ခေါ်ဆိုမှုကို ခေါ်ဆိုမှု", "通話記錄"
                                     };

const char *call_dialing_title[] = {"拨号中", "Dialing in", "ダイヤル中", "에서 전화 걸기", "Wählen", "Commis en ligne", "Marcar", "Τηλεφωνώ.", \
                                    "Наберите номер", "Na discagem", "โทรออก", "La composizione in", "الاتصال الهاتفي", "Arama", "تمرین", "Wybieranie numeru", \
                                    "Dial", "में डायलिंग", "Inbellen", "Quay số", "חיוג ב", "Dail", "ချိန်း", "撥號中"
                                   };

const char *record_tip_title[] = {"是否恢复出厂设置？", "Restore factory settings?", "工場出荷時の設定に戻しますか?", "공장 설정을 복원 하시겠습니까?", "Werden die Werks einstellungen wieder hergestellt?", "Voulez-vous restaurer les paramètres d'usine?", "¿Se restablece la configuración de fábrica?", "Επαναφορά ρυθμίσεων εργοστασίου;", \
                                  "Вы восстановили заводские настройки?", "As configurações de fábrica são restauradas?", "จะคืนค่าการตั้งค่าจากโรงงานหรือไม่?", "Ripristinare le impostazioni di fabbrica?", "هل ستستأنف إعدادات المصنع ؟", "Fabrika ayarlarına geri yüklenir mi?", "بازگردانیدن تنظیمات کارخانه ؟", "Przywrócić ustawienia fabryczne?", \
                                  "Apakah akan kembali ke pengaturan pabrik?", "कारखाना सेटिंग बहाल करें?", "Fabrieksinstellingen herstellen?", "Cài đặt gốc có được khôi phục không?", "לשחזר את הגדרות המפעל?", "Adakah untuk memulihkan tetapan kilang?", "တည်ဆောက်ချက်ကို ပြန်လည်ပြန်ထားခြင်း?", "是否恢復出廠設置?"
                                 };

const char *shoutdown_tip_title[] = {"是否关机？", "Shutdown?", "電源を切りますか?", "셧다운?", "Ist es herunter gefahren?", "Est-il éteint?", "¿Está apagado?", "Κλείσιμο;", \
                                     "Это выключено?", "Está desligado?", "Lo spegnimento?", "هل هو مغلق ؟", "رسالة", "Kapalı mı?", "تعطيل؟", "Zamknięcie?", \
                                     "Apakah dimatikan?", "बंद?", "Afsluiten?", "Nó có tắt không?", "כיבוי?", "Adakah ia dimatikan?", "ပိတ်ထားပါ?", "是否關機?"
                                    };

const char *reset_tip_title[] = {"是否重启？", "Restart?", "再起動しますか?", "다시 시작?", "Wird es neu gestartet?", "Est-ce un redémarrage?", "¿Se reinicia?", "Επανεκκίνηση;", \
                                 "Это перезагрузка?", "É reiniciado?", "รีสตาร์ทหรือไม่?", "Ripartire?", "هل إعادة التشغيل ؟", "Yeniden başlatılıyor mu?", "آغاز؟", "Ponowne uruchomienie?", \
                                 "Apakah akan restart?", "पुनरारंभ करें?", "Opnieuw opstarten?", "Có khởi động lại không?", "הפעל מחדש?", "Adakah ia dimulakan semula?", "ပြန်လည်လုပ်ပါ?", "是否重啟?"
                                };

const char *massage_title[] = {"消息", "Message", "メッセージ", "메시지", "Nachricht", "Messages", "Mensaje", "Μήνυμα", \
                               "Сообщения", "Mensagens", "ข้อความ", "Messaggio", "رسالة", "Mesaj", "پیام", "Wiadomość", \
                               "Pesan", "संदेश", "Bericht", "Tin nhắn", "הודעה", "Mesej", "အကြောင်းကြားစာ -", "消息"
                              };

const char *blue_touch_title[] = {"蓝牙名称：", "Bluetooth Name:", "ブルートゥース名:", "블루투스 이름:", "Bluetooth Name:", "Nom Bluetooth:", "Nombre de Bluetooth:", "Όνομα Bluetooth:", \
                                  "Название Bluetooth:", "Nome do Bluetooth:", "ชื่อบลูทูธ:", "Nome Bluetooth:", "اسم بلوتوث:", "Bluetooth adı:", "نام بلوتوث:", "Nazwa Bluetooth:", \
                                  "Nama Bluetooth:", "ब्लूटूथ नाम:", "Bluetooth Naam:", "Tên Bluetooth:", "שם Bluetooth:", "Nama Bluetooth:", "Bluetooth အမည် -", "藍牙名稱:"
                                 };

const char *version_title[] = {"版本信息：", "Version information:", "バージョン情報:", "버전 정보:", "Versions informationen:", "Informations sur la version:", "Información de la versión:", "Πληροφορίες έκδοσης:", \
                               "Информация о версии:", "Informações da versão:", "ข้อมูลรุ่น:", "Informazioni sulla versione:", "معلومات الإصدار:", "Sürüm Bilgisi:", "اطلاعات نسخه:", "Informacje o wersji:", \
                               "Informasi Versi:", "संस्करण की जानकारी:", "Versie informatie:", "Thông tin phiên bản:", "פרטי גרסה:", "Maklumat versi:", "ဗားရှင်းအချက်အလက် -", "版本信息:"
                              };

const char *device_info_title[] = {"设备信息", "Device Information", "設備情報", "장치 정보", "Geräte informationen", "Informations sur l'appareil", "Información del dispositivo", "Πληροφορίες συσκευής", \
                                   "Информация об устройстве", "Informações do dispositivo", "ข้อมูลอุปกรณ์", "Informazioni sul dispositivo", "معلومات الجهاز", "Cihaz Bilgisi", "اطلاعات دستگاه", "Informacje o urządzeniu", \
                                   "Informasi perangkat", "युक्ति जानकारी", "Apparaatinformatie", "TThông tin thiết bị", "פרטי המכשיר", "Maklumat peranti", "အကောင့်အချက်အလက်များ", "設備信息"
                                  };

const char *boot_language_title[] = {"开机语言", "Boot language", "起動言語", "부팅 언어", "Boot-Sprache", "Langue de démarrage", "Lenguaje de arranque", "Γλώσσα εκκίνησης", \
                                     "Язык запуска", "Linguagem de inicialização", "ภาษาบูต", "Linguaggio di avvio", "لغة التشغيل", "Önyükleme dili", "زبان راه اندازی", "Język rozruchowy", \
                                     "Bahasa boot", "बूट भाषा", "Boot taal", "Ngôn ngữ khởi động", "שפת האתחול", "Bahasa but", "စကားစု", "開機語言"
                                    };

const char *control_photo_title[] = {"遥控拍照", "Remote control photo", "リモコン撮影", "원격 제어 사진", "Fernbedienungs-Kamera", "Prenez des photos à distance", "Foto con control remoto", "Φωτογραφία απομακρυσμένου χειρισμού", \
                                     "Пульт дистанционного управления для фотографирования", "Tire fotos remotamente", "กล้องควบคุมระยะไกล", "Foto del telecomando", "التقاط الصور عن بعد", "Uzaktan kamera", "عکس کنترل دوردست", "Zdalne sterowanie zdjęciem", \
                                     "Kamera remote control", "रिमोट कंट्रोल फोटो", "Afstandsbediening foto", "Chụp ảnh từ xa", "תמונת שלט רחוק", "Kamera kawalan jauh", "အဝေး ထိန်းချုပ် ချိန်ပုံစံ", "遙控拍照"
                                    };


#endif

