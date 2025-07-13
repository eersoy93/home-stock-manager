#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock.h"
#include "resource.h"
#include "theme.h"

// Window and control IDs
#define ID_LISTVIEW     1001
#define ID_BTN_ADD      1002
#define ID_BTN_EDIT     1003
#define ID_BTN_DELETE   1004
#define ID_BTN_SAVE     1005
#define ID_BTN_LOAD     1006
#define ID_MENU_FILE    1007
#define ID_MENU_ABOUT   1008

// Global variables
HWND hMainWindow;
HWND hListView;
HWND hBtnAdd, hBtnEdit, hBtnDelete, hBtnSave, hBtnLoad;
HINSTANCE hInst;
StockManager stockManager;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMainWindow(void);
void CreateControls(HWND hwnd);
void InitializeListView(void);
void RefreshListView(void);
void ShowAddItemDialogWrapper(void);
void ShowEditItemDialogWrapper(int index);
void DeleteSelectedItem(void);
void SaveStockData(void);
void LoadStockData(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance; // Suppress unused parameter warning
    (void)lpCmdLine;     // Suppress unused parameter warning
    
    hInst = hInstance;
    
    // Set UTF-8 for MinGW-w64
    #ifdef __MINGW32__
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Initialize theme
    InitTheme(&g_theme, THEME_MODE_LIGHT);
    
    // Initialize stock manager
    InitStockManager(&stockManager);
    
    // Auto-load stock data on startup
    LoadStockFromFile(&stockManager, "stock_data.dat");
    
    // Create main window
    CreateMainWindow();
    
    // Show window
    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    FreeStockManager(&stockManager);
    FreeTheme(&g_theme);
    
    return (int)msg.wParam;
}

void CreateMainWindow(void)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"StockManagerWindow";
    wc.hbrBackground = g_theme.hBackgroundBrush;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClass(&wc);
    
    hMainWindow = CreateWindowEx(
        0,
        L"StockManagerWindow",
        L"Home Stock Manager - Modern Theme",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 650,
        NULL, NULL, hInst, NULL
    );
}

void CreateControls(HWND hwnd)
{
    // Create ListView with modern styling
    hListView = CreateWindowEx(
        0,
        WC_LISTVIEW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        20, 20, 680, 580,
        hwnd, (HMENU)ID_LISTVIEW, hInst, NULL
    );
    
    // Create themed buttons with larger size and better spacing
    hBtnAdd = CreateWindow(L"BUTTON", L"➕ Add Product", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                          720, 20, 140, 40, hwnd, (HMENU)ID_BTN_ADD, hInst, NULL);
    
    hBtnEdit = CreateWindow(L"BUTTON", L"✏️ Edit Product", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           720, 70, 140, 40, hwnd, (HMENU)ID_BTN_EDIT, hInst, NULL);
    
    hBtnDelete = CreateWindow(L"BUTTON", L"🗑️ Delete Product", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                             720, 120, 140, 40, hwnd, (HMENU)ID_BTN_DELETE, hInst, NULL);
    
    hBtnSave = CreateWindow(L"BUTTON", L"💾 Save Data", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           720, 200, 140, 40, hwnd, (HMENU)ID_BTN_SAVE, hInst, NULL);
    
    hBtnLoad = CreateWindow(L"BUTTON", L"📁 Load Data", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           720, 250, 140, 40, hwnd, (HMENU)ID_BTN_LOAD, hInst, NULL);
    
    InitializeListView();
    
    // Apply modern theme to all controls
    ApplyThemeToListView(hListView, &g_theme);
    ApplyThemeToButton(hBtnAdd, BUTTON_TYPE_PRIMARY, &g_theme);
    ApplyThemeToButton(hBtnEdit, BUTTON_TYPE_WARNING, &g_theme);
    ApplyThemeToButton(hBtnDelete, BUTTON_TYPE_DANGER, &g_theme);
    ApplyThemeToButton(hBtnSave, BUTTON_TYPE_SUCCESS, &g_theme);
    ApplyThemeToButton(hBtnLoad, BUTTON_TYPE_SECONDARY, &g_theme);
}

void InitializeListView(void)
{
    // Set ListView columns
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    
    // Product Name column
    lvc.iSubItem = 0;
    lvc.pszText = L"Product Name";
    lvc.cx = 250;
    ListView_InsertColumn(hListView, 0, &lvc);
    
    // Stock Quantity column
    lvc.iSubItem = 1;
    lvc.pszText = L"Quantity";
    lvc.cx = 100;
    ListView_InsertColumn(hListView, 1, &lvc);
    
    // Category column
    lvc.iSubItem = 2;
    lvc.pszText = L"Category";
    lvc.cx = 200;
    ListView_InsertColumn(hListView, 2, &lvc);
}

void RefreshListView(void)
{
    // Clear ListView
    ListView_DeleteAllItems(hListView);
    
    // Add stock items
    for (int i = 0; i < stockManager.itemCount; i++)
    {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        
        // Product name - convert to wide string
        wchar_t wname[MAX_NAME_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, stockManager.items[i].name, -1, wname, MAX_NAME_LENGTH);
        lvi.pszText = wname;
        ListView_InsertItem(hListView, &lvi);
        
        // Stock quantity
        wchar_t wstock[32];
        swprintf(wstock, 32, L"%d", stockManager.items[i].stock);
        ListView_SetItemText(hListView, i, 1, wstock);
        
        // Category - convert to wide string
        wchar_t wcategory[MAX_CATEGORY_LENGTH];
        MultiByteToWideChar(CP_UTF8, 0, stockManager.items[i].category, -1, wcategory, MAX_CATEGORY_LENGTH);
        ListView_SetItemText(hListView, i, 2, wcategory);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            CreateControls(hwnd);
            // Refresh ListView with auto-loaded data
            RefreshListView();
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_BTN_ADD:
                    ShowAddItemDialogWrapper();
                    break;
                    
                case ID_BTN_EDIT:
                    {
                        int selected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                        if (selected != -1)
                            ShowEditItemDialogWrapper(selected);
                        else
                            ThemedMessageBox(hwnd, L"⚠️ Please select an item to edit.", L"Warning", MB_OK | MB_ICONWARNING);
                    }
                    break;
                    
                case ID_BTN_DELETE:
                    DeleteSelectedItem();
                    break;
                    
                case ID_BTN_SAVE:
                    SaveStockData();
                    break;
                    
                case ID_BTN_LOAD:
                    LoadStockData();
                    break;
            }
            break;
            
        case WM_SIZE:
            // Resize controls when window size changes
            if (hListView)
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                
                // Calculate button area - buttons are 140px wide + 20px margin
                int buttonAreaWidth = 180;
                int listViewWidth = width - buttonAreaWidth - 40;
                int buttonX = width - 160;
                
                // Resize ListView with padding
                SetWindowPos(hListView, NULL, 20, 20, listViewWidth, height - 60, SWP_NOZORDER);
                
                // Reposition themed buttons
                if (hBtnAdd) SetWindowPos(hBtnAdd, NULL, buttonX, 20, 140, 40, SWP_NOZORDER);
                if (hBtnEdit) SetWindowPos(hBtnEdit, NULL, buttonX, 70, 140, 40, SWP_NOZORDER);
                if (hBtnDelete) SetWindowPos(hBtnDelete, NULL, buttonX, 120, 140, 40, SWP_NOZORDER);
                if (hBtnSave) SetWindowPos(hBtnSave, NULL, buttonX, 200, 140, 40, SWP_NOZORDER);
                if (hBtnLoad) SetWindowPos(hBtnLoad, NULL, buttonX, 250, 140, 40, SWP_NOZORDER);
            }
            break;
            
        case WM_DESTROY:
            // Auto-save stock data on exit
            SaveStockToFile(&stockManager, "stock_data.dat");
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ShowAddItemDialogWrapper(void)
{
    ShowAddItemDialog(hMainWindow, &stockManager);
    RefreshListView();
}

void ShowEditItemDialogWrapper(int index)
{
    ShowEditItemDialog(hMainWindow, &stockManager, index);
    RefreshListView();
}

void DeleteSelectedItem(void)
{
    int selected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (selected != -1)
    {
        int result = ThemedMessageBox(hMainWindow, L"❓ Are you sure you want to delete the selected item?", 
                               L"Delete Confirmation", MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES)
        {
            RemoveStockItem(&stockManager, selected);
            RefreshListView();
        }
    }
    else
    {
        ThemedMessageBox(hMainWindow, L"⚠️ Please select an item to delete.", L"Warning", MB_OK | MB_ICONWARNING);
    }
}

void SaveStockData(void)
{
    if (SaveStockToFile(&stockManager, "stock_data.dat"))
    {
        ThemedMessageBox(hMainWindow, L"✅ Stock data saved successfully.", L"Information", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        ThemedMessageBox(hMainWindow, L"❌ Error occurred while saving stock data.", L"Error", MB_OK | MB_ICONERROR);
    }
}

void LoadStockData(void)
{
    if (LoadStockFromFile(&stockManager, "stock_data.dat"))
    {
        RefreshListView();
        ThemedMessageBox(hMainWindow, L"✅ Stock data loaded successfully.", L"Information", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        ThemedMessageBox(hMainWindow, L"❌ Error occurred while loading stock data.\nFile may not exist or be corrupted.", L"Error", MB_OK | MB_ICONERROR);
    }
}
