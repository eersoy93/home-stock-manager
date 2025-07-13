#ifndef THEME_H
#define THEME_H

#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>

// Modern theme colors
#define THEME_COLOR_BACKGROUND      RGB(248, 249, 250)
#define THEME_COLOR_SURFACE         RGB(255, 255, 255)
#define THEME_COLOR_PRIMARY         RGB(0, 123, 255)
#define THEME_COLOR_PRIMARY_HOVER   RGB(0, 86, 179)
#define THEME_COLOR_SUCCESS         RGB(40, 167, 69)
#define THEME_COLOR_SUCCESS_HOVER   RGB(33, 136, 56)
#define THEME_COLOR_DANGER          RGB(220, 53, 69)
#define THEME_COLOR_DANGER_HOVER    RGB(176, 42, 55)
#define THEME_COLOR_WARNING         RGB(255, 193, 7)
#define THEME_COLOR_WARNING_HOVER   RGB(227, 172, 6)
#define THEME_COLOR_SECONDARY       RGB(108, 117, 125)
#define THEME_COLOR_SECONDARY_HOVER RGB(90, 98, 104)
#define THEME_COLOR_BORDER          RGB(222, 226, 230)
#define THEME_COLOR_TEXT            RGB(33, 37, 41)
#define THEME_COLOR_TEXT_MUTED      RGB(108, 117, 125)

// Dark theme colors
#define THEME_COLOR_DARK_BACKGROUND RGB(33, 37, 41)
#define THEME_COLOR_DARK_SURFACE    RGB(52, 58, 64)
#define THEME_COLOR_DARK_BORDER     RGB(73, 80, 87)
#define THEME_COLOR_DARK_TEXT       RGB(248, 249, 250)

// Button types
typedef enum {
    BUTTON_TYPE_PRIMARY,
    BUTTON_TYPE_SUCCESS,
    BUTTON_TYPE_DANGER,
    BUTTON_TYPE_WARNING,
    BUTTON_TYPE_SECONDARY
} ButtonType;

// Theme mode
typedef enum {
    THEME_MODE_LIGHT,
    THEME_MODE_DARK
} ThemeMode;

// Button state
typedef enum {
    BUTTON_STATE_NORMAL,
    BUTTON_STATE_HOVER,
    BUTTON_STATE_PRESSED
} ButtonState;

// Theme structure
typedef struct {
    ThemeMode mode;
    COLORREF backgroundColor;
    COLORREF surfaceColor;
    COLORREF textColor;
    COLORREF borderColor;
    HFONT hFont;
    HFONT hFontBold;
    HBRUSH hBackgroundBrush;
    HBRUSH hSurfaceBrush;
    HPEN hBorderPen;
} Theme;

// Custom button data
typedef struct {
    ButtonType type;
    ButtonState state;
    BOOL isHovered;
    WNDPROC originalProc;
} ButtonData;

// Function prototypes
void InitTheme(Theme* theme, ThemeMode mode);
void FreeTheme(Theme* theme);
void ApplyThemeToWindow(HWND hwnd, Theme* theme);
void ApplyThemeToButton(HWND hwnd, ButtonType type, Theme* theme);
void ApplyThemeToListView(HWND hwnd, Theme* theme);
void ApplyThemeToDialog(HWND hwnd, Theme* theme);

// Custom MessageBox with emoji support
int ThemedMessageBox(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType);

COLORREF GetButtonColor(ButtonType type, ButtonState state);
LRESULT CALLBACK ThemedButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawThemedButton(HWND hwnd, HDC hdc, ButtonType type, ButtonState state, RECT* rect, const wchar_t* text);
void DrawGradientRect(HDC hdc, RECT* rect, COLORREF color1, COLORREF color2, BOOL vertical);

// Global theme instance
extern Theme g_theme;

#endif // THEME_H
