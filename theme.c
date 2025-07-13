#include "theme.h"
#include <commctrl.h>

// Global theme instance
Theme g_theme;

void InitTheme(Theme* theme, ThemeMode mode)
{
    if (theme == NULL) return;
    
    theme->mode = mode;
    
    // Set colors based on theme mode
    if (mode == THEME_MODE_DARK) {
        theme->backgroundColor = THEME_COLOR_DARK_BACKGROUND;
        theme->surfaceColor = THEME_COLOR_DARK_SURFACE;
        theme->textColor = THEME_COLOR_DARK_TEXT;
        theme->borderColor = THEME_COLOR_DARK_BORDER;
    } else {
        theme->backgroundColor = THEME_COLOR_BACKGROUND;
        theme->surfaceColor = THEME_COLOR_SURFACE;
        theme->textColor = THEME_COLOR_TEXT;
        theme->borderColor = THEME_COLOR_BORDER;
    }
    
    // Create fonts
    theme->hFont = CreateFont(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    
    theme->hFontBold = CreateFont(
        -14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    
    // Create brushes and pens
    theme->hBackgroundBrush = CreateSolidBrush(theme->backgroundColor);
    theme->hSurfaceBrush = CreateSolidBrush(theme->surfaceColor);
    theme->hBorderPen = CreatePen(PS_SOLID, 1, theme->borderColor);
}

void FreeTheme(Theme* theme)
{
    if (theme == NULL) return;
    
    if (theme->hFont) DeleteObject(theme->hFont);
    if (theme->hFontBold) DeleteObject(theme->hFontBold);
    if (theme->hBackgroundBrush) DeleteObject(theme->hBackgroundBrush);
    if (theme->hSurfaceBrush) DeleteObject(theme->hSurfaceBrush);
    if (theme->hBorderPen) DeleteObject(theme->hBorderPen);
    
    memset(theme, 0, sizeof(Theme));
}

COLORREF GetButtonColor(ButtonType type, ButtonState state)
{
    switch (type) {
        case BUTTON_TYPE_PRIMARY:
            return (state == BUTTON_STATE_HOVER) ? THEME_COLOR_PRIMARY_HOVER : THEME_COLOR_PRIMARY;
        case BUTTON_TYPE_SUCCESS:
            return (state == BUTTON_STATE_HOVER) ? THEME_COLOR_SUCCESS_HOVER : THEME_COLOR_SUCCESS;
        case BUTTON_TYPE_DANGER:
            return (state == BUTTON_STATE_HOVER) ? THEME_COLOR_DANGER_HOVER : THEME_COLOR_DANGER;
        case BUTTON_TYPE_WARNING:
            return (state == BUTTON_STATE_HOVER) ? THEME_COLOR_WARNING_HOVER : THEME_COLOR_WARNING;
        case BUTTON_TYPE_SECONDARY:
        default:
            return (state == BUTTON_STATE_HOVER) ? THEME_COLOR_SECONDARY_HOVER : THEME_COLOR_SECONDARY;
    }
}

void DrawGradientRect(HDC hdc, RECT* rect, COLORREF color1, COLORREF color2, BOOL vertical)
{
    GRADIENT_RECT gRect = {0, 1};
    TRIVERTEX vertices[2];
    
    // First vertex
    vertices[0].x = rect->left;
    vertices[0].y = rect->top;
    vertices[0].Red = GetRValue(color1) << 8;
    vertices[0].Green = GetGValue(color1) << 8;
    vertices[0].Blue = GetBValue(color1) << 8;
    vertices[0].Alpha = 0;
    
    // Second vertex
    vertices[1].x = rect->right;
    vertices[1].y = rect->bottom;
    vertices[1].Red = GetRValue(color2) << 8;
    vertices[1].Green = GetGValue(color2) << 8;
    vertices[1].Blue = GetBValue(color2) << 8;
    vertices[1].Alpha = 0;
    
    GradientFill(hdc, vertices, 2, &gRect, 1, 
                 vertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

void DrawThemedButton(HWND hwnd, HDC hdc, ButtonType type, ButtonState state, RECT* rect, const wchar_t* text)
{
    (void)hwnd; // Suppress unused parameter warning
    
    COLORREF buttonColor = GetButtonColor(type, state);
    COLORREF textColor = RGB(255, 255, 255);
    
    // Create lighter color for gradient
    COLORREF gradientColor = RGB(
        min(255, GetRValue(buttonColor) + 20),
        min(255, GetGValue(buttonColor) + 20),
        min(255, GetBValue(buttonColor) + 20)
    );
    
    // Draw gradient background
    DrawGradientRect(hdc, rect, gradientColor, buttonColor, TRUE);
    
    // Draw border
    HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(
        max(0, GetRValue(buttonColor) - 30),
        max(0, GetGValue(buttonColor) - 30),
        max(0, GetBValue(buttonColor) - 30)
    ));
    
    HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    RoundRect(hdc, rect->left, rect->top, rect->right, rect->bottom, 6, 6);
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBorderPen);
    
    // Draw text
    if (text && wcslen(text) > 0) {
        SetTextColor(hdc, textColor);
        SetBkMode(hdc, TRANSPARENT);
        
        HFONT hOldFont = (HFONT)SelectObject(hdc, g_theme.hFont);
        
        DrawText(hdc, text, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
    }
}

LRESULT CALLBACK ThemedButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ButtonData* buttonData = (ButtonData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_MOUSEMOVE:
            if (!buttonData->isHovered) {
                buttonData->isHovered = TRUE;
                buttonData->state = BUTTON_STATE_HOVER;
                InvalidateRect(hwnd, NULL, FALSE);
                
                // Track mouse leave
                TRACKMOUSEEVENT tme = {0};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
            }
            break;
            
        case WM_MOUSELEAVE:
            if (buttonData->isHovered) {
                buttonData->isHovered = FALSE;
                buttonData->state = BUTTON_STATE_NORMAL;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
            
        case WM_LBUTTONDOWN:
            buttonData->state = BUTTON_STATE_PRESSED;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
            
        case WM_LBUTTONUP:
            buttonData->state = buttonData->isHovered ? BUTTON_STATE_HOVER : BUTTON_STATE_NORMAL;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
            
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            wchar_t text[256];
            GetWindowText(hwnd, text, 256);
            
            DrawThemedButton(hwnd, hdc, buttonData->type, buttonData->state, &rect, text);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            free(buttonData);
            break;
    }
    
    return CallWindowProc(buttonData->originalProc, hwnd, uMsg, wParam, lParam);
}

void ApplyThemeToButton(HWND hwnd, ButtonType type, Theme* theme)
{
    if (hwnd == NULL || theme == NULL) return;
    
    // Create button data
    ButtonData* buttonData = (ButtonData*)malloc(sizeof(ButtonData));
    if (buttonData == NULL) return;
    
    buttonData->type = type;
    buttonData->state = BUTTON_STATE_NORMAL;
    buttonData->isHovered = FALSE;
    buttonData->originalProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ThemedButtonProc);
    
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)buttonData);
    
    // Set font
    SendMessage(hwnd, WM_SETFONT, (WPARAM)theme->hFont, TRUE);
    
    // Force redraw
    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyThemeToWindow(HWND hwnd, Theme* theme)
{
    if (hwnd == NULL || theme == NULL) return;
    
    // Set window background
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)theme->hBackgroundBrush);
    
    // Force redraw
    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyThemeToListView(HWND hwnd, Theme* theme)
{
    if (hwnd == NULL || theme == NULL) return;
    
    // Set ListView colors
    ListView_SetBkColor(hwnd, theme->surfaceColor);
    ListView_SetTextBkColor(hwnd, theme->surfaceColor);
    ListView_SetTextColor(hwnd, theme->textColor);
    
    // Set extended styles for modern appearance
    DWORD exStyle = ListView_GetExtendedListViewStyle(hwnd);
    exStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
    ListView_SetExtendedListViewStyle(hwnd, exStyle);
    
    // Set font
    SendMessage(hwnd, WM_SETFONT, (WPARAM)theme->hFont, TRUE);
    
    // Force redraw
    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyThemeToDialog(HWND hwnd, Theme* theme)
{
    if (hwnd == NULL || theme == NULL) return;
    
    // Set dialog background
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)theme->hSurfaceBrush);
    
    // Apply theme to all child controls
    HWND hChild = GetWindow(hwnd, GW_CHILD);
    while (hChild != NULL) {
        wchar_t className[256];
        GetClassName(hChild, className, 256);
        
        if (wcscmp(className, L"Static") == 0) {
            // Apply theme to static controls (labels)
            SendMessage(hChild, WM_SETFONT, (WPARAM)theme->hFont, TRUE);
        } else if (wcscmp(className, L"Edit") == 0) {
            // Apply theme to edit controls
            SendMessage(hChild, WM_SETFONT, (WPARAM)theme->hFont, TRUE);
        }
        
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
    
    // Force redraw
    InvalidateRect(hwnd, NULL, TRUE);
}

// Custom MessageBox with emoji support
int ThemedMessageBox(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType)
{
    // Use MessageBoxW with proper Unicode handling
    // Set thread locale to ensure proper emoji rendering
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
    
    // Create message box with Unicode support
    int result = MessageBoxW(hWnd, lpText, lpCaption, uType | MB_SETFOREGROUND);
    
    // Restore original settings
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT));
    
    return result;
}
