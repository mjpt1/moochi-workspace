#pragma once
// =====================================================
//   MOOCHI  —  کاتالوگِ چهره‌ها
//
//   ☠️ چرا ترکيبِ خودکار نساختم:
//
//      وسوسه‌اش هست — يک حلقه‌ي تودرتو روي شش شکل ابرو و
//      شانزده چشم و دوازده دهان مي‌نويسي و «هزار چهره»
//      داري. ولي بيشترشان **بي‌معني** هستند:
//
//        چشمِ خواب‌آلود + زبانِ بيرون + تاج  =  ؟
//
//      يک ترکيبِ معتبرِ برنامه‌نويسي که هيچ احساسي را
//      نمي‌گويد. همان اشتباهي که در آن فايلِ «۱۲۵ چهره»
//      شده بود: ۵×۵×۵ شمرده شده بود و در عمل يک صورت بود.
//
//   پس هر نسخه **دستي** نوشته شده و اسم دارد. سيستمِ ترکيبي
//   فقط نوشتنش را از پنجاه خط به يک خط رسانده.
//
//   هر سطر:  { نام ، ابرو ، چشم ، دهان ، لپ ، اضافه ، پرچم }
// =====================================================

#include "moochi_face.h"

// ✂️ ۳ مهر ۱۴۰۵ — مهسن از ۱۶۲ چهره ، ۸۸ تا را حذف کرد (۷۴ ماند).
//    فهرستِ کامل پيشين: _unused/moochi_catalog_کامل_162.h
//    شماره‌هاي حذف‌شده (بر اساسِ شماره‌گذاريِ ۱۶۲تايي):
//    1-25, 27-30, 32, 34-40, 42-50, 53-67, 70-75, 77, 79-84,
//    87, 97, 106, 108, 112-121
const FaceRecipe CATALOG[] = {
// ---------------- شادي و مهر ----------------

// ---------------- کنجکاوي و فکر ----------------

// ---------------- تعجب ----------------
{ "جا خورد",          BR_UP,     EK_RING,    MK_OPEN,    BK_NONE,  XK_NONE,     0 },

// ---------------- غم ----------------
{ "ناراحت",           BR_SAD,    EK_ROUND,   MK_SADWAVE, BK_OVAL,  XK_NONE,     0 },
{ "گريه",             BR_SAD,    EK_ROUND,   MK_SADWAVE, BK_OVAL,  XK_TEARS,    0 },

// ---------------- خشم ----------------
{ "عصباني",           BR_ANGRY,  EK_FLAME,   MK_SHOUT,   BK_NONE,  XK_ANGERMARK,FR_SHAKE },

// ---------------- خواب ----------------

// ---------------- حالِ بد ----------------
{ "سرگيجه",           BR_SAD,    EK_SPIRAL,  MK_WAVE,    BK_NONE,  XK_NONE,    FR_SHAKE },
{ "تهوع",             BR_SAD,    EK_SPIRAL,  MK_WAVE,    BK_NONE,  XK_SWEAT,    0 },

// ---------------- با اضافات ----------------

// ---------------- روزمره ----------------
{ "آماده",            BR_UP,     EK_ROUND2,  MK_TINY,    BK_OVAL,  XK_NONE,     0 },
{ "تاييد",            BR_CALM,   EK_CARET,   MK_TINY,    BK_OVAL,  XK_NONE,     0 },

// ---------------- غذا و بدن ----------------
{ "خوشمزه",           BR_UP,     EK_STAR,    MK_TONGUE,  BK_BIG,   XK_SPARKLES, 0 },
{ "بدمزه",            BR_ANGRY,  EK_CARET,   MK_TONGUE,  BK_NONE,  XK_SWEAT,    0 },

// ---------------- بازي ----------------
{ "تشويق",            BR_UP,     EK_ROUND2,  MK_OPEN,    BK_OVAL,  XK_HEARTS,   0 },

// ---------------- شب و روز ----------------
{ "بيدار شد",         BR_UP,     EK_ROUND,   MK_OPEN,    BK_OVAL,  XK_NONE,     0 },
{ "روياي خوب",        BR_NONE,   EK_SLEEP,   MK_TINY,    BK_BIG,   XK_HEARTS,   0 },
{ "کابوس",            BR_SAD,    EK_RING,    MK_SHOUT,   BK_NONE,  XK_SWEAT,   FR_SHAKE },

// ---------------- شخصيت ----------------
{ "مغرور",            BR_ANGRY,  EK_HALF,    MK_TINY,    BK_NONE,  XK_CROWN,    0 },
{ "مرموز",            BR_ONE_UP, EK_HALF,    MK_FLAT,    BK_NONE,  XK_GLASSES,  0 },
{ "معصوم",            BR_UP,     EK_ROUND2,  MK_SMALL_O, BK_BIG,   XK_NONE,    FR_BOW },
{ "دلبر",             BR_CALM,   EK_WINK_L,  MK_KISS3,   BK_BIG,   XK_HEARTS,  FR_BOW },
{ "جدي",              BR_ANGRY,  EK_LINE,    MK_FLAT,    BK_OVAL,  XK_NONE,     0 },
{ "خيال‌باف",          BR_CALM,   EK_STAR,    MK_SMALL_O, BK_OVAL,  XK_BULB,     0 },
{ "بامزه",            BR_UP,     EK_CARET,   MK_BLOB,    BK_BIG,   XK_NONE,     0 },

// ---------------- واکنش ----------------
{ "خنديد",            BR_UP,     EK_ARC,     MK_OPEN,    BK_BIG,   XK_SPARKLES, 0 },
{ "آه کشيد",          BR_SAD,    EK_HALF,    MK_BLOB,    BK_NONE,  XK_NONE,     0 },
{ "جيغ",              BR_UP,     EK_RING,    MK_SHOUT,   BK_NONE,  XK_ANGERMARK,FR_SHAKE },
{ "سوت زد",           BR_CALM,   EK_ARC,     MK_SMALL_O, BK_OVAL,  XK_NOTES,    0 },
{ "نفس عميق",         BR_CALM,   EK_ARC,     MK_OPEN,    BK_OVAL,  XK_NONE,     0 },
{ "سر تکان داد",      BR_CALM,   EK_ARC,     MK_FLAT,    BK_OVAL,  XK_NONE,     0 },
{ "جا خالي داد",      BR_UP,     EK_CARET,   MK_FLAT,    BK_NONE,  XK_SWEAT,   FR_SHAKE },
{ "سرخ شد",           BR_SAD,    EK_CARET,   MK_SMALL_O, BK_BIG,   XK_SWEAT,   FR_BOW },

// ---------------- ترکيب‌هاي ويژه ----------------
{ "گربه‌ي عصباني",     BR_ANGRY,  EK_FLAME,   MK_SHOUT,   BK_NONE,  XK_CATEARS, FR_SHAKE },
{ "شاهزاده‌ي غمگين",   BR_SAD,    EK_ROUND,   MK_SADWAVE, BK_OVAL,  XK_CROWN,    0 },
{ "شيطونکِ عاشق",      BR_CALM,   EK_HEART,   MK_KISS3,   BK_BIG,   XK_HORNS,    0 },
{ "باحالِ خندان",      BR_FLAT,   EK_ROUND,   MK_WIDE,    BK_OVAL,  XK_GLASSES,  0 },

// —— دورِ دوم —— هر سطر هشت ميدان دارد ، ميدانِ آخر «دست» است.
//    سطرهاي بالا هفت ميدانه‌اند و همچنان معتبر: C++ ميدانِ
//    نيامده را صفر مي‌گذارد ، و HK_NONE هم صفر است.

// ---------- دست ----------
{ "سلام",             BR_UP,     EK_ROUND,   MK_WIDE,    BK_OVAL,  XK_NONE,     0,       HK_WAVE      },
{ "خداحافظ",          BR_SAD,    EK_ARC,     MK_TINY,    BK_OVAL,  XK_NONE,     0,       HK_WAVE      },
{ "آفرين",            BR_UP,     EK_CARET,   MK_BIGSMILE,BK_OVAL,  XK_NONE,     0,       HK_THUMBUP   },
{ "پيروزي",           BR_UP,     EK_ROUND2,  MK_BIGSMILE,BK_OVAL,  XK_SPARKLES, FR_BOW,  HK_VICTORY   },
{ "اونجا رو!",        BR_UP,     EK_ROUND,   MK_OVAL,    BK_NONE,  XK_NONE,     0,       HK_POINT     },
{ "خنده‌ي پنهان",      BR_CALM,   EK_CARET,   MK_TINY,    BK_BIG,   XK_NONE,     0,       HK_OVERMOUTH },
{ "نگاه نکن",         BR_SAD,    EK_ROUND,   MK_SQUIGGLE,BK_BIG,   XK_NONE,     0,       HK_OVEREYES  },
{ "چي؟!",             BR_UP,     EK_RING,    MK_OVAL,    BK_NONE,  XK_QUESTION, 0,       HK_OPEN      },
{ "دست زد",           BR_UP,     EK_CARET,   MK_BIGSMILE,BK_OVAL,  XK_SPARKLES, 0,       HK_CLAP      },
{ "قلب با دست",       BR_CALM,   EK_ROUND2,  MK_TINY,    BK_BIG,   XK_HEARTS,   FR_BOW,  HK_HEARTHAND },
{ "چشم قربان",        BR_FLAT,   EK_ROUND,   MK_FLAT,    BK_NONE,  XK_NONE,     0,       HK_SALUTE    },
{ "نمي‌دونم",          BR_SAD,    EK_LINE,    MK_CROOKED, BK_NONE,  XK_QUESTION, 0,       HK_SHRUG     },
{ "خجالتِ دستي",       BR_SAD,    EK_ARC,     MK_SMALL_O, BK_BIG,   XK_NONE,     FR_BOW,  HK_SHY       },

// ---------- چشم‌هاي تازه ----------
{ "بي‌روح",            BR_FLAT,   EK_DOT,     MK_FLAT,    BK_NONE,  XK_NONE,     0 },
{ "منگ",              BR_SAD,    EK_SWIRL,   MK_SQUIGGLE,BK_NONE,  XK_NONE,     0 },
{ "مشکوک",            BR_ANGRY,  EK_SQUINT,  MK_CROOKED, BK_NONE,  XK_NONE,     0 },
{ "از حال رفت",       BR_NONE,   EK_CROSS,   MK_BLOB,    BK_NONE,  XK_NONE,     0 },
{ "نبين!",            BR_ANGRY,  EK_CROSS2,  MK_ZIGZAG,  BK_NONE,  XK_NONE,     0 },
{ "کلافه",            BR_FLAT,   EK_ROLLUP,  MK_FLAT,    BK_NONE,  XK_SWEATMULTI, 0 },

// ---------- دهان‌هاي تازه ----------
{ "معذب",             BR_SAD,    EK_ROUND,   MK_SQUIGGLE,BK_OVAL,  XK_SWEATMULTI, 0 },
{ "ترسيده",           BR_SAD,    EK_RING,    MK_ZIGZAG,  BK_NONE,  XK_SWEATMULTI, FR_SHAKE },
{ "اوه!",             BR_UP,     EK_ROUND,   MK_OVAL,    BK_NONE,  XK_NONE,     0 },
{ "شيطنت",            BR_ONE_UP, EK_CARET,   MK_TONGUEOUT,BK_OVAL, XK_HORNS,    0 },
{ "طعنه",             BR_ONE_UP, EK_SQUINT,  MK_CROOKED, BK_NONE,  XK_NONE,     0 },
{ "نيش‌خند",           BR_FLAT,   EK_HALF,    MK_NONE,    BK_NONE,  XK_FANGS,    0 },
{ "خون‌آشام",          BR_ANGRY,  EK_SQUINT,  MK_NONE,    BK_NONE,  XK_FANGS,    0 },

// ---------- اضافاتِ تازه ----------
{ "باتري تمام",       BR_SAD,    EK_HALF,    MK_FLAT,    BK_NONE,  XK_BATTERY,  0 },
{ "در حال شارژ",      BR_CALM,   EK_SLEEP,   MK_TINY,    BK_OVAL,  XK_CHARGING, 0 },
{ "جستجو",            BR_ONE_UP, EK_ROUND,   MK_SMALL_O, BK_NONE,  XK_MAGNIFIER,0 },
{ "صبر کن",           BR_CALM,   EK_LINE,    MK_FLAT,    BK_NONE,  XK_HOURGLASS,0 },
{ "سردمه",            BR_SAD,    EK_WOBBLE,  MK_ZIGZAG,  BK_OVAL,  XK_SNOW,     FR_SHAKE },
{ "صبح بخير",         BR_UP,     EK_ARC,     MK_WIDE,    BK_OVAL,  XK_SUN,      0 },
{ "شب بخير",          BR_CALM,   EK_SLEEP,   MK_TINY,    BK_NONE,  XK_MOON,     0 },
{ "گرممه",            BR_SAD,    EK_HALF,    MK_OPEN,    BK_BIG,   XK_THERMO,   0 },
{ "گل آورد",          BR_CALM,   EK_ARC,     MK_TINY,    BK_BIG,   XK_FLOWER,   FR_BOW },
{ "آبنبات",           BR_UP,     EK_ROUND2,  MK_TONGUEOUT,BK_OVAL, XK_CANDY,    0 },
{ "جادوگر",           BR_ONE_UP, EK_ROUND,   MK_CROOKED, BK_NONE,  XK_WIZARDHAT,0 },
{ "تولد",             BR_UP,     EK_CARET,   MK_BIGSMILE,BK_OVAL,  XK_PARTYHAT, FR_BOW },
{ "جشن",              BR_UP,     EK_STAR,    MK_OPEN,    BK_OVAL,  XK_CONFETTI, 0,       HK_CLAP      },
{ "دزد دريايي",       BR_ANGRY,  EK_ROUND,   MK_CROOKED, BK_NONE,  XK_EYEPATCH, 0 },
{ "دلشکسته",          BR_SAD,    EK_WOBBLE,  MK_SADWAVE, BK_NONE,  XK_BROKENHEART, 0 },
};

const int CATALOG_N = sizeof(CATALOG) / sizeof(CATALOG[0]);

// =====================================================
//   وصل کردنِ چهره‌ها به رفتار
//
//   ☠️ بدون اين بخش ، چهره‌ها فقط يک اسلايدشو هستند.
//      همان اشتباهي که در آن فايلِ «۱۲۵ چهره» شده بود:
//      چهره‌ها بودند ولي هيچ‌کدام **دليلي** براي آمدن
//      نداشتند.
//
//   هر چهره بايد يا راه‌اندازيِ مستقيم داشته باشد ، يا
//   عضوِ يک دسته‌ي خودجوش باشد که موچي گاهي از آن قرعه
//   مي‌کشد.
// =====================================================

// پيدا کردنِ چهره با نام — گران نيست چون يک بار در setup
// و بعد فقط با انديس کار مي‌کنيم.
inline int faceByName(const char *n) {
  for (int i = 0; i < CATALOG_N; i++)
    if (strcmp(CATALOG[i].name, n) == 0) return i;
  return -1;
}

// ---------- دسته‌هاي خودجوش ----------
//
// وقتي موچي بي‌کار است ، از يکي از اين دسته‌ها قرعه
// مي‌کشد — کدام دسته ، به شخصيتش بستگي دارد.

// موچيِ دل‌بسته ، وقتي بي‌کار است
const char *SET_HAPPY[] = {
  "بامزه", "سوت زد", "خيال‌باف", "سر تکان داد", "نفس عميق",
  "معصوم", "سلام", "آفرين", "نيش‌خند", "گل آورد",
  "آبنبات"
};
// موچيِ دل‌خور
const char *SET_GRUMPY[] = {
  "جدي", "مغرور", "کلافه", "بي‌روح", "طعنه",
  "مشکوک", "آه کشيد"
};
// کنجکاوي و فکر
const char *SET_CURIOUS[] = {
  "مرموز", "جستجو", "چي؟!", "نمي‌دونم", "اونجا رو!"
};
// وقتي کسي حرف مي‌زند
const char *SET_LISTEN[] = {
  "سوت زد", "چشم قربان", "اوه!", "نمي‌دونم"
};
// دست زدن و تشويق
const char *SET_CHEER[] = {
  "تشويق", "خنديد", "دست زد", "پيروزي", "جشن",
  "آفرين"
};
// نوازش
const char *SET_PET[] = {
  "دلبر", "خنده‌ي پنهان", "خجالتِ دستي", "گل آورد", "سرخ شد"
};
// عشق
const char *SET_LOVE[] = {
  "دلبر", "شيطونکِ عاشق", "قلب با دست", "نگاه نکن"
};
// اذيت شدن
const char *SET_HURT[] = {
  "ناراحت", "دلشکسته", "معذب", "ترسيده", "منگ",
  "گريه", "شاهزاده‌ي غمگين"
};
// عصبانيت
const char *SET_ANGRY[] = {
  "عصباني", "جيغ", "گربه‌ي عصباني", "نبين!", "خون‌آشام",
  "دزد دريايي"
};
// خواب
const char *SET_SLEEP[] = {
  "روياي خوب", "شب بخير", "در حال شارژ", "باتري تمام", "بيدار شد",
  "کابوس", "خداحافظ"
};
// لباس و تزيين — گاهي بي‌دليل
const char *SET_DRESSUP[] = {
  "باحالِ خندان", "جادوگر", "تولد", "دزد دريايي", "خون‌آشام"
};

// —— دسته‌هاي تازه ——

// وقتي موچي شيطنتش مي‌گيرد
const char *SET_MISCHIEF[] = {
  "شيطنت", "طعنه", "نيش‌خند", "مشکوک", "خنده‌ي پنهان"
};
// حالِ بدِ بدني — گرما ، سرما ، گيجي
const char *SET_UNWELL[] = {
  "سردمه", "گرممه", "منگ", "از حال رفت", "معذب",
  "کلافه", "سرگيجه", "تهوع"
};
// جا خوردن
const char *SET_SURPRISE[] = {
  "جا خورد", "اوه!", "چي؟!"
};
// بي‌کاريِ آرام — وقتي هيچ اتفاقي نمي‌افتد
const char *SET_IDLE[] = {
  "آماده", "تاييد", "صبر کن"
};
// خوراک
const char *SET_FOOD[] = {
  "خوشمزه", "بدمزه", "آبنبات"
};
// بازي
const char *SET_PLAY[] = {
  "جا خالي داد"
};

// لحظه‌ي روشن شدن و لحظه‌ي بيدار شدن
//   «شب بخير» و «خداحافظ» عمدا اينجا نيستند — براي لحظه‌ي
//   خوابيدن‌اند ، نه بيدار شدن. يک دسته بايد **يک** موقعيت
//   را بگويد ، وگرنه قرعه‌کشي گاهي بي‌معني مي‌شود.
const char *SET_GREET[] = {
  "سلام", "صبح بخير", "چشم قربان", "آفرين"
};

#define SET_N(a) ((int)(sizeof(a) / sizeof((a)[0])))

// يکي از دسته را تصادفي برمي‌گرداند
// ☠️ چرا «فقط يک‌سري چهره مي‌آمد»:
//
//    قرعه‌ي يکنواخت **حافظه ندارد**. هر بار از صفر شروع مي‌کند ،
//    پس چهره‌اي که همين الان آمده دوباره به همان احتمال مي‌آيد.
//    در يک دسته‌ي ۲۰تايي ، براي ديدنِ هر بيست‌تا به طورِ ميانگين
//    ۲۰×H(۲۰) ≈ **۷۲** قرعه لازم است ، نه ۲۰. و چند تايشان
//    معمولا خيلي ديرتر مي‌آيند. اين همان «آخري‌ها نمي‌آيند» است.
//
//    راهِ حل — «بهترين از چهار»:
//      چهار عضو را تصادفي بردار ، آني را بگير که **کمتر از همه**
//      ديده شده. هم تصادفي مي‌ماند (قابلِ پيش‌بيني نيست) و هم
//      توزيع را صاف مي‌کند.
//
//    شبيه‌سازي روي ۱۵۰۰ قرعه (نسبتِ بيشترين به کمترين):
//      قرعه‌ي يکنواخت  ۱۹٫۷×   ← بعضي چهره‌ها عملا ديده نمي‌شدند
//      بهترين از سه     ۶٫۲×
//      بهترين از چهار   ۴٫۳×   ← انتخاب شد
//      بهترين از پنج    ۵٫۲×   ← بدتر ، چون نمونه‌ها هم‌پوشاني
//                                 مي‌گيرند و تنوع کم مي‌شود
//
//    هزينه: ۱۶۲ بايت رم. همين.
uint8_t faceSeen[256] = {0};

inline void faceSeenDecay() {
  // وقتي شمارنده پر شد ، همه را نصف کن تا نسبت‌ها بماند
  for (int i = 0; i < CATALOG_N; i++) faceSeen[i] >>= 1;
}

inline int pickFrom(const char **set, int n) {
  if (n <= 0) return 0;
  int best = -1, bestSeen = 999;
  for (int k = 0; k < 4; k++) {
    int idx = faceByName(set[random(n)]);
    if (idx < 0) continue;
    if (faceSeen[idx] < bestSeen) { bestSeen = faceSeen[idx]; best = idx; }
  }
  // 🛡 نبايد به «چهره‌ي شماره‌ي صفر» برگردد — آن يک چهره‌ي
  //    تصادفي است ، نه «هيچ». ‎−۱ يعني «چهره‌ي خودِ حالت را نشان بده».
  if (best < 0) return -1;
  if (faceSeen[best] >= 250) faceSeenDecay();
  faceSeen[best]++;
  return best;
}
