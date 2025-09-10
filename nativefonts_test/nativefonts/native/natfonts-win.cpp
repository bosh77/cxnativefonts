
#include <fstream>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <string>
#include <windows.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <commdlg.h>

#include <sstream>


using namespace std;

// Struttura per gestire i font OpenGL
struct FontOpenGL {
    GLuint base;           // Base delle display list
    int height;            // Altezza del font
    HDC hdc;              // Device context per la creazione del font
    HFONT hFont;          // Handle del font
    bool isValid;         // Flag di validità
    
    FontOpenGL() : base(0), height(0), hdc(NULL), hFont(NULL), isValid(false) {}
};


class BBNativeFonts : public Object{
public:
	

	BBNativeFonts();
	
	~BBNativeFonts();
	
	bool _New( String fontname, int dimensione, int peso, bool ital );
	
	bool _DrawText( String text, int x, int y, int jj, int c );

    int _GetTextWidth( String text );
	int _GetTextHeight( String text );

    int _SelectFont();
    
    String _GetSize( String txt);

   
    static String _GetFontListString();

private:



struct FontEnumData {
    vector<string> fontNames;
};

// String _GetFontListString();
static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam);
static int CALLBACK EnumFontFamProc(LOGFONT* lpLogFont, TEXTMETRIC* lpTextMetric, DWORD FontType, LPARAM lParam);


char *convertBBString(String string);

void ARGBtoFloats(uint32_t argb, float& a, float& r, float& g, float& b);

	// signed char *_data;
	String _fontname;
    
    int _dimensione;
    int _peso;
    bool _ital;

    FontOpenGL _font;

};

// ***** BBNativeFonts.cpp *****

BBNativeFonts::BBNativeFonts():_fontname(""){
}

BBNativeFonts::~BBNativeFonts(){
	// if( _data ) free( _data );
}

String BBNativeFonts::_GetSize(String txt){
    // Usa lo stesso DC per consistenza
    HDC hdc = GetDC(GetActiveWindow());
    
    // if (!hdc) {
        // hdc=GetDC(HWND_DESKTOP);
        // return "0@0"; // Gestione errore
    // }
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, _font.hFont);
    
    string testo = convertBBString(txt);
    
    // METODO 1: Usa GetTextExtentPoint32A (consistente con altre funzioni)
    // SIZE size;
    // if (GetTextExtentPoint32A(hdc, testo.c_str(), (int)testo.length(), &size)) {
    //     int largTesto = size.cx;
    //     int altezzaTesto = size.cy;
        
    //     // Ripristina il font precedente
    //     SelectObject(hdc, hOldFont);
    //     ReleaseDC(GetActiveWindow(), hdc);
        
    //     //??????
    //     altezzaTesto=MulDiv(altezzaTesto,72, GetDeviceCaps(hdc, LOGPIXELSY));

    //     String fs = String(largTesto) + "@" + String(altezzaTesto);
        
    //     return fs;
    // }
    
    // METODO 2: Se preferisci DrawTextA, usa questo:
    
    RECT rect = {0, 0, 0, 0};
    int altezza = DrawTextA(hdc, testo.c_str(), -1, &rect, DT_CALCRECT | DT_SINGLELINE);
    
    int altezzaTesto = rect.bottom - rect.top;
    int largTesto = rect.right - rect.left;
    
    
    // Ripristina e rilascia sempre
    SelectObject(hdc, hOldFont);
    ReleaseDC(GetActiveWindow(), hdc);
    
    //??????
    // altezzaTesto=MulDiv(altezzaTesto,72, GetDeviceCaps(hdc, LOGPIXELSY));

    //??????
    altezzaTesto = altezzaTesto *0.8;

    String fs = String(largTesto) + "@" + String(altezzaTesto);
        
    return fs;

    return "0@0"; // Fallback in caso di errore
}

bool BBNativeFonts::_New( String fontname, int dimensione, int peso, bool ital ){

	// printf("OK1\n");
	_fontname=fontname;
    _dimensione=dimensione;
    _peso=peso;
    _ital=ital;

    _dimensione = -MulDiv(dimensione,72, GetDeviceCaps(GetDC(GetActiveWindow()), LOGPIXELSY));

    // printf(_fontname);

    // String _f2=&_fontname;
    // string _f3=_f2.Copy();

    string nomeFonte = convertBBString(_fontname);

    // Ottieni il device context corrente
    // _font.hdc = wglGetCurrentDC();
    _font.hdc = GetDC(GetActiveWindow());

    if (!_font.hdc) {
        printf("ERRORE: Impossibile ottenere il device context\n");
        return -1;
    }
    
    // int dimensione = 64;  // Dimensione più piccola per test
    

    //https://learn.microsoft.com/it-it/windows/win32/api/wingdi/nf-wingdi-createfonta


    // 4. ABILITA il controllo errori per il font
    _font.hFont = CreateFontA(
        _dimensione,                 // Altezza
        0,                         // Larghezza
        0, 0,                      // Angoli
        peso,                 // Peso
        ital,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // Strikeout
        ANSI_CHARSET,              // Charset
        OUT_TT_PRECIS,             // Output precision
        CLIP_DEFAULT_PRECIS,          // Clipping
        ANTIALIASED_QUALITY,          // Quality
        DEFAULT_PITCH | FF_DONTCARE,   // Family
        nomeFonte.c_str()              // Nome del font
    );
    

    // 5. CONTROLLA se il font è stato creato
    if (!_font.hFont) {
        printf("ERRORE: Impossibile creare il font %s\n", (const char*)_fontname.Data());
        return false;
    }


    HFONT oldFont = (HFONT)SelectObject(_font.hdc, _font.hFont);

// Genera le display list
    _font.base = glGenLists(224);
    _font.height = dimensione;
    


    //wglUseFontOutlines


    
    // 6. CONTROLLA se wglUseFontBitmaps ha successo
    if (!wglUseFontBitmaps(_font.hdc, 32, 224, _font.base)) {
        printf("ERRORE: wglUseFontBitmaps fallito\n");
        glDeleteLists(_font.base, 224);
        SelectObject(_font.hdc, oldFont);
        DeleteObject(_font.hFont);
        return -1;
    }
    
    // Ripristina il font precedente
    // SelectObject(font.hdc, oldFont);
    _font.isValid = true;
    
    //  printf("OK1\n");

	return true;

}

bool BBNativeFonts::_DrawText( String text, int x, int y, int jj, int c ){
    
    String WH = _GetSize(text);

    string WH2 = convertBBString(WH);

    std::vector<std::string> parti;

    std::stringstream ss(WH2);
    std::string item;

    while (std::getline(ss, item, '@')) {
        parti.push_back(item);
    }

    int W=stoi(parti[0]);

    int H=stoi(parti[1]);
    
    int jx=jj & 3;
    int jy=jj & 12;

    int xx;
    int yy;

    switch (jx) {
        case 0:
        xx=x;
        break;

        case 1:
        xx=x-W/2;
        break;

        case 2:
        xx=x-W;
        break;
    }

    switch (jy) {
        case 0:
        yy=y+H;
        break;

        case 4:
        yy=y+H/2;
        break;

        case 8:
        yy=y;
        break;
    }


    float a, r, g, b;

    ARGBtoFloats(c,a,r,g,b);

    // 7. CONFIGURA la matrice di proiezione per coordinate 2D
    // glMatrixMode(GL_PROJECTION);
    // glPushMatrix();
    // glLoadIdentity();
    
    // Ottieni le dimensioni della finestra
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];
    
    glPushMatrix();

    glUseProgram(0);

    //int width = 800;
    //int height = 600;

    // Imposta coordinate 2D (0,0) in alto a sinistra
    glOrtho(0, width, height, 0, -1, 1);
    
    // glMatrixMode(GL_MODELVIEW);
    // glPushMatrix();
    // glLoadIdentity();
    
    // 8. DISABILITA depth test per il testo
    // glDisable(GL_DEPTH_TEST);
    
    // 9. IMPOSTA colore visibile (rosso per test)
    glColor4f(r, g, b, a);  // ROSSO per essere sicuri che sia visibile
    
    
    // 10. USA coordinate sicuramente visibili
    glRasterPos2f(xx, yy);  // Vicino all'angolo in alto a sinistra
    
    // Imposta la base delle display list
    glListBase(_font.base - 32);
    
    // string text2 = "TEST OPENGL VISIBILE!";
    
    // 11. AGGIUNGI debug per verificare la posizione
    // printf("Rendering testo: %s alle coordinate (10, 50)\n", testo.c_str());
    
    // string testo = "TEST OPENGL VISIBILE!";

    // Renderizza il testo
    string testo = convertBBString(text);

    // glCallLists(text.Length(), GL_UNSIGNED_BYTE, text.Data());
    
    // glRotatef(35, 1.0f, 1.0f, 0.0f);

    glCallLists(testo.length(), GL_UNSIGNED_BYTE, testo.c_str());
    
    

    // 12. FORZA il flush per assicurarsi che venga disegnato
    glFlush();
    
    glPopMatrix();


    return true;
    
}




// RECT rect = {0, 0, 0, 0};
// const char* testo = "Il mio testo di esempio";

// // Calcola le dimensioni senza disegnare
// int altezza = DrawTextA(hdc, testo, -1, &rect, DT_CALCRECT | DT_SINGLELINE);

// // rect.bottom conterrà l'altezza del testo
// int altezzaTesto = rect.bottom - rect.top;




int BBNativeFonts::_GetTextWidth( String text ){

    //https://learn.microsoft.com/en-us/windows/win32/gdi/font-and-text-functions

    
    // HDC hdc = CreateCompatibleDC(NULL);
    // HDC hdc = GetDC(HWND_DESKTOP);
    HDC hdc = GetDC(GetActiveWindow());

    SelectObject(hdc, _font.hFont);
    
    string testo = convertBBString(text);

    // std::string testo = "Ciao GetTextExtentPoint32A!";

    SIZE size;

    GetTextExtentPoint32A(hdc, testo.c_str(), (int)testo.length(), &size);

    // if (GetTextExtentPoint32A(hdc, testo.c_str(), (int)testo.length(), &size)) {
    //     printf("Testo: '%s'\n", testo.c_str());
    //     printf("Larghezza: %d px, Altezza: %d px\n", size.cx, size.cy);
    // } else {
    //     printf("Errore in GetTextExtentPoint32A\n");
    // }



    // int nHeight = -MulDiv(50, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    // return nHeight;

    return size.cx;

}
int BBNativeFonts::_GetTextHeight( String text ){

    HDC hdc = GetDC(GetActiveWindow());

    SelectObject(hdc, _font.hFont);
    
    string testo = convertBBString(text);

    SIZE size;

    GetTextExtentPoint32A(hdc, testo.c_str(), (int)testo.length(), &size);

    return size.cy;

}

int BBNativeFonts::_SelectFont(){

    //https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfonta

    //https://learn.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-choosefonta
    
    //HWND_DESKTOP

    CHOOSEFONT cf;
    LOGFONT lf;
    HFONT hfont;
    
    lf = {0};

    ZeroMemory(&lf, sizeof(lf));

    // lf.lfWeight = -_dimensione;
    // lf.lfFaceName[LF_FACESIZE]=(char)*"Vineta BT";

    string fname = convertBBString(_fontname);

    memcpy( (LPSTR)&lf.lfFaceName, fname.c_str(), fname.length()+1 );

// Initialize LOGFONT
HDC hdc = GetDC(GetActiveWindow());
// ZeroMemory(&lf, sizeof(lf));
lf.lfHeight           = -MulDiv(_dimensione, GetDeviceCaps(hdc, LOGPIXELSY), 72);
lf.lfWidth            = 0;
lf.lfEscapement       = 0;
lf.lfOrientation      = 0;
lf.lfWeight           = _peso;
lf.lfItalic           = _ital;
lf.lfUnderline        = FALSE;
lf.lfStrikeOut        = FALSE;
lf.lfCharSet          = DEFAULT_CHARSET;
lf.lfOutPrecision     = OUT_DEFAULT_PRECIS;
lf.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
lf.lfQuality          = DEFAULT_QUALITY;
lf.lfPitchAndFamily   = DEFAULT_PITCH | FF_DONTCARE;

    // HWND hwndDesktop = FindWindow("Progman", NULL);
    // HWND hwndX = FindWindow("BBGlfwGame", "Native Fonts and Printer Demo");

    // HWND hActiveWindow = GetActiveWindow();
    // SetForegroundWindow(hActiveWindow);

    ZeroMemory(&cf, sizeof(cf));
    

    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = GetActiveWindow();
    cf.lpLogFont = &lf;
    // cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
    cf.Flags = CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_TTONLY;;

// SetWindowPos(HWND_DESKTOP, HWND_BOTTOM, 0, 0, 0, 0,
//              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    if (ChooseFont(&cf)) {

        
// BringWindowToBottom(HWND_DESKTOP);
        // HWND hDlg = FindWindow("BBGlfwGame", "Tipo di carattere");
        //   HWND hDlg = FindWindow("BBGlfwGame", NULL); // Classe generica per dialog  


//           HWND hDlg = NULL;
// HWND hWnd = NULL;

// while ((hWnd = FindWindowEx(NULL, hWnd, "#32770", NULL)) != NULL) {
//     char *title;
//     if (GetWindowText(hWnd, (LPSTR)title, sizeof(title)/sizeof(wchar_t))) {
//         // if (wcsstr(title, "carattere") || wcsstr(title, "Font")) {
//         if (title=="Tipo di carattere:"){
//             hDlg = hWnd;
//             break;
//         }
//     }
// }




    //   if (hDlg) {
    //       SetForegroundWindow(hDlg);
    //       BringWindowToTop(hDlg);
          
    //   }else{exit;}
        
        
        hfont = CreateFontIndirect(&lf);

        // esempio: applico font a una static
        // SendMessage(HWND_DESKTOP, WM_SETFONT, (WPARAM)hfont, TRUE);

        // _font.hFont = hfont;

        // return lf.lfWeight;

    _fontname=lf.lfFaceName;
    // _dimensione=lf.lfHeight*0.72;
    
    
    _dimensione=-MulDiv(lf.lfHeight,72, GetDeviceCaps(hdc, LOGPIXELSY));


    
    _peso=lf.lfWeight;
    _ital=lf.lfItalic;
    
    String fontname=_fontname;
    int dimensione=_dimensione;
    int peso=_peso;
    bool ital=_ital;

    string nomeFonte = convertBBString(_fontname);

    // Ottieni il device context corrente
    // _font.hdc = wglGetCurrentDC();
    _font.hdc = GetDC(GetActiveWindow());

    if (!_font.hdc) {
        printf("ERRORE: Impossibile ottenere il device context\n");
        return -1;
    }
    
    // int dimensione = 64;  // Dimensione più piccola per test
    
    // 4. ABILITA il controllo errori per il font
    _font.hFont = CreateFontA(
        -_dimensione,                 // Altezza
        0,                         // Larghezza
        0, 0,                      // Angoli
        peso,                 // Peso
        ital,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // Strikeout
        ANSI_CHARSET,              // Charset
        OUT_TT_PRECIS,             // Output precision
        CLIP_DEFAULT_PRECIS,          // Clipping
        ANTIALIASED_QUALITY,          // Quality
        DEFAULT_PITCH | FF_DONTCARE,   // Family
        nomeFonte.c_str()              // Nome del font
    );
    
    // 5. CONTROLLA se il font è stato creato
    if (!_font.hFont) {
        printf("ERRORE: Impossibile creare il font %s\n", (const char*)_fontname.Data());
        return false;
    }


    HFONT oldFont = (HFONT)SelectObject(_font.hdc, _font.hFont);

// Genera le display list
    _font.base = glGenLists(224);
    _font.height = dimensione;
    
    // 6. CONTROLLA se wglUseFontBitmaps ha successo
    if (!wglUseFontBitmaps(_font.hdc, 32, 224, _font.base)) {
        printf("ERRORE: wglUseFontBitmaps fallito\n");
        glDeleteLists(_font.base, 224);
        SelectObject(_font.hdc, oldFont);
        DeleteObject(_font.hFont);
        return -1;
    }
    
    // Ripristina il font precedente
    // SelectObject(font.hdc, oldFont);
    _font.isValid = true;



    }

    return 0;

}
    
// String BBNativeFonts::_GetSize() {



// }

char *BBNativeFonts::convertBBString(String string){
  int srclength = string.Length();
  char *result = (char *)malloc(srclength+1);
  std::wstring string_ws(string.Data(), srclength);
  wcstombs(result, string_ws.c_str(), srclength+1);
  return result;
}


void BBNativeFonts::ARGBtoFloats(uint32_t argb, float& a, float& r, float& g, float& b) {
    a = ((argb >> 24) & 0xFF) / 255.0f;
    r = ((argb >> 16) & 0xFF) / 255.0f;
    g = ((argb >> 8)  & 0xFF) / 255.0f;
    b = ((argb >> 0)  & 0xFF) / 255.0f;
}


 // Nuova funzione GetFontListString per Cerberus X
// Cambia anche l'implementazione in statica
String BBNativeFonts::_GetFontListString() {
    static string result;
    FontEnumData enumData;
    
    // printf("Inizio enumerazione font...\n");
    
    HDC hdc = GetDC(NULL);
    if (hdc == NULL) {
        printf("ERRORE: Impossibile ottenere device context\n");
        return "";
    }
    
    // Usa EnumFontFamilies che enumera solo i nomi delle famiglie
    int result_enum = EnumFontFamilies(hdc, NULL, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&enumData);
    
    // printf("EnumFontFamilies risultato: %d\n", result_enum);
    // printf("Font trovati: %zu\n", enumData.fontNames.size());
    
    ReleaseDC(NULL, hdc);
    
    // Costruisci la stringa
    result = "";
    for (size_t i = 0; i < enumData.fontNames.size(); i++) {
        if (i > 0) {
            result += ",";
        }
        result += enumData.fontNames[i];
    }
    
    if (result.empty()) {
        result = "Arial,Times New Roman,Courier New,Verdana,Tahoma";
    }
    
    // return "A,B,C";
    
    return String(result.c_str());
    
}

// Callback function per EnumFontFamiliesEx
int CALLBACK BBNativeFonts::EnumFontFamExProc(
    ENUMLOGFONTEX* lpelfe,
    NEWTEXTMETRICEX* lpntme,
    DWORD FontType,
    LPARAM lParam
) {
    FontEnumData* data = (FontEnumData*)lParam;
    
    if (!data) {
        printf("ERRORE: data è NULL nel callback\n");
        return 0;
    }
    
    // Converti il nome del font
    string fontName;
    
    // Usa sempre la versione ANSI per semplicità
    fontName = string((char*)lpelfe->elfLogFont.lfFaceName);
    
    printf("Font trovato: %s\n", fontName.c_str());
    
    // Aggiungi il nome del font alla lista (evita duplicati)
    bool found = false;
    for (const auto& name : data->fontNames) {
        if (name == fontName) {
            found = true;
            break;
        }
    }
    
    if (!found && !fontName.empty()) {
        // data->fontNames.push_back(fontName);
        // printf("Font aggiunto: %s (totale: %zu)\n", fontName.c_str(), data->fontNames.size());
    }
    
    return 1; // Continua l'enumerazione
}


// Funzione che restituisce una stringa con i font separati da virgole
// String BBNativeFonts::fontnat(){
    
// }

// Nuovo callback per EnumFontFamilies
int CALLBACK BBNativeFonts::EnumFontFamProc(
    LOGFONT* lpLogFont,
    TEXTMETRIC* lpTextMetric,
    DWORD FontType,
    LPARAM lParam
) {
    FontEnumData* data = (FontEnumData*)lParam;
    
    if (!data) return 0;
    
    string fontName = string((char*)lpLogFont->lfFaceName);
    
    // Controlla duplicati
    bool found = false;
    for (const auto& name : data->fontNames) {
        if (name == fontName) {
            found = true;
            break;
        }
    }
    
    if (!found && !fontName.empty()) {
        data->fontNames.push_back(fontName);
        // printf("Font famiglia aggiunta: %s\n", fontName.c_str());
    }
    
    return 1;
}






