#include "stock.h"
#include "resource.h"
#include "theme.h"
#include <commctrl.h>

// UTF-8 validation function
int IsValidUTF8(const char* str)
{
    if (str == NULL) return 0;
    
    const unsigned char* bytes = (const unsigned char*)str;
    while (*bytes)
    {
        if ((*bytes & 0x80) == 0) // ASCII
        {
            bytes++;
        }
        else if ((*bytes & 0xE0) == 0xC0) // 2-byte UTF-8
        {
            if ((bytes[1] & 0xC0) != 0x80) return 0;
            bytes += 2;
        }
        else if ((*bytes & 0xF0) == 0xE0) // 3-byte UTF-8
        {
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return 0;
            bytes += 3;
        }
        else if ((*bytes & 0xF8) == 0xF0) // 4-byte UTF-8
        {
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return 0;
            bytes += 4;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

// Safe UTF-8 string copy
void SafeUTF8Copy(char* dest, const char* src, size_t destSize)
{
    if (dest == NULL || src == NULL || destSize == 0) return;
    
    if (IsValidUTF8(src))
    {
        strncpy(dest, src, destSize - 1);
        dest[destSize - 1] = '\0';
    }
    else
    {
        // If invalid UTF-8, copy as much as possible
        size_t len = strlen(src);
        if (len >= destSize) len = destSize - 1;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
}

// Global variables
StockManager* g_stockManager = NULL;
int g_editIndex = -1;
HWND g_hMainWindow = NULL;

void InitStockManager(StockManager* manager)
{
    if (manager == NULL) return;
    
    // Set console to UTF-8 for MinGW-w64
    #ifdef __MINGW32__
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    
    manager->itemCount = 0;
    manager->nextId = 1;
    memset(manager->items, 0, sizeof(manager->items));
}

void FreeStockManager(StockManager* manager)
{
    if (manager == NULL) return;
    
    // In this implementation, we don't use dynamic memory
    // We only reset the counter
    manager->itemCount = 0;
}

int AddStockItem(StockManager* manager, const char* name, const char* category, int stock)
{
    if (manager == NULL || name == NULL || category == NULL) return 0;
    if (manager->itemCount >= MAX_ITEMS) return 0;
    if (stock < 0) return 0;
    
    StockItem* item = &manager->items[manager->itemCount];
    
    SafeUTF8Copy(item->name, name, MAX_NAME_LENGTH);
    SafeUTF8Copy(item->category, category, MAX_CATEGORY_LENGTH);
    
    item->stock = stock;
    item->id = manager->nextId++;
    
    manager->itemCount++;
    return 1;
}

int RemoveStockItem(StockManager* manager, int index)
{
    if (manager == NULL || index < 0 || index >= manager->itemCount) return 0;
    
    // Remove item (shift)
    for (int i = index; i < manager->itemCount - 1; i++)
    {
        manager->items[i] = manager->items[i + 1];
    }
    
    manager->itemCount--;
    return 1;
}

int UpdateStockItem(StockManager* manager, int index, const char* name, const char* category, int stock)
{
    if (manager == NULL || index < 0 || index >= manager->itemCount) return 0;
    if (name == NULL || category == NULL) return 0;
    if (stock < 0) return 0;
    
    StockItem* item = &manager->items[index];
    
    SafeUTF8Copy(item->name, name, MAX_NAME_LENGTH);
    SafeUTF8Copy(item->category, category, MAX_CATEGORY_LENGTH);
    
    item->stock = stock;
    
    return 1;
}

int FindStockItem(StockManager* manager, const char* name)
{
    if (manager == NULL || name == NULL) return -1;
    
    for (int i = 0; i < manager->itemCount; i++)
    {
        if (strcmp(manager->items[i].name, name) == 0)
            return i;
    }
    
    return -1;
}

void SortStockItems(StockManager* manager, int sortBy)
{
    if (manager == NULL || manager->itemCount <= 1) return;
    
    // Simple bubble sort
    for (int i = 0; i < manager->itemCount - 1; i++)
    {
        for (int j = 0; j < manager->itemCount - i - 1; j++)
        {
            int shouldSwap = 0;
            
            switch (sortBy)
            {
                case 0: // Name
                    shouldSwap = strcmp(manager->items[j].name, manager->items[j + 1].name) > 0;
                    break;
                case 1: // Stock
                    shouldSwap = manager->items[j].stock > manager->items[j + 1].stock;
                    break;
                case 2: // Category
                    shouldSwap = strcmp(manager->items[j].category, manager->items[j + 1].category) > 0;
                    break;
            }
            
            if (shouldSwap)
            {
                StockItem temp = manager->items[j];
                manager->items[j] = manager->items[j + 1];
                manager->items[j + 1] = temp;
            }
        }
    }
}

int SaveStockToFile(StockManager* manager, const char* filename)
{
    if (manager == NULL || filename == NULL) return 0;
    
    FILE* file = fopen(filename, "wb");
    if (file == NULL) return 0;
    
    // Write binary header
    fwrite(&manager->itemCount, sizeof(int), 1, file);
    fwrite(&manager->nextId, sizeof(int), 1, file);
    
    // Write all items in binary format
    for (int i = 0; i < manager->itemCount; i++)
    {
        StockItem* item = &manager->items[i];
        fwrite(&item->id, sizeof(int), 1, file);
        fwrite(item->name, sizeof(char), MAX_NAME_LENGTH, file);
        fwrite(item->category, sizeof(char), MAX_CATEGORY_LENGTH, file);
        fwrite(&item->stock, sizeof(int), 1, file);
    }
    
    fclose(file);
    return 1;
}

int LoadStockFromFile(StockManager* manager, const char* filename)
{
    if (manager == NULL || filename == NULL) return 0;
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return 0;
    
    int itemCount, nextId;
    
    // Read binary header
    if (fread(&itemCount, sizeof(int), 1, file) != 1 || 
        fread(&nextId, sizeof(int), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }
    
    if (itemCount < 0 || itemCount > MAX_ITEMS)
    {
        fclose(file);
        return 0;
    }
    
    manager->itemCount = 0;
    manager->nextId = nextId;
    
    // Read all items in binary format
    for (int i = 0; i < itemCount; i++)
    {
        StockItem* item = &manager->items[i];
        
        if (fread(&item->id, sizeof(int), 1, file) != 1 ||
            fread(item->name, sizeof(char), MAX_NAME_LENGTH, file) != MAX_NAME_LENGTH ||
            fread(item->category, sizeof(char), MAX_CATEGORY_LENGTH, file) != MAX_CATEGORY_LENGTH ||
            fread(&item->stock, sizeof(int), 1, file) != 1)
        {
            break;
        }
        
        // Ensure null termination for strings
        item->name[MAX_NAME_LENGTH - 1] = '\0';
        item->category[MAX_CATEGORY_LENGTH - 1] = '\0';
        
        manager->itemCount++;
    }
    
    fclose(file);
    return 1;
}

void SearchStockItems(StockManager* manager, const char* searchTerm, StockItem* results, int* resultCount)
{
    if (manager == NULL || searchTerm == NULL || results == NULL || resultCount == NULL) return;
    
    *resultCount = 0;
    
    for (int i = 0; i < manager->itemCount; i++)
    {
        if (strstr(manager->items[i].name, searchTerm) != NULL ||
            strstr(manager->items[i].category, searchTerm) != NULL)
        {
            results[*resultCount] = manager->items[i];
            (*resultCount)++;
        }
    }
}

int GetLowStockItems(StockManager* manager, int threshold, StockItem* results, int* resultCount)
{
    if (manager == NULL || results == NULL || resultCount == NULL) return 0;
    
    *resultCount = 0;
    
    for (int i = 0; i < manager->itemCount; i++)
    {
        if (manager->items[i].stock <= threshold)
        {
            results[*resultCount] = manager->items[i];
            (*resultCount)++;
        }
    }
    
    return 1;
}

// Dialog functions
void ShowAddItemDialog(HWND parent, StockManager* manager)
{
    g_stockManager = manager;
    g_editIndex = -1;
    g_hMainWindow = parent;
    
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_ITEM), parent, AddEditItemDialogProc);
}

void ShowEditItemDialog(HWND parent, StockManager* manager, int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= manager->itemCount) return;
    
    g_stockManager = manager;
    g_editIndex = itemIndex;
    g_hMainWindow = parent;
    
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_ITEM), parent, AddEditItemDialogProc);
}

INT_PTR CALLBACK AddEditItemDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    (void)lParam; // Suppress unused parameter warning
    
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Set dialog title
            SetWindowText(hDlg, g_editIndex >= 0 ? L"✏️ Edit Product" : L"➕ Add New Product");
            
            // Apply modern theme to dialog
            ApplyThemeToDialog(hDlg, &g_theme);
            
            // Update static text labels with emojis
            HWND hNameLabel = GetDlgItem(hDlg, IDC_STATIC_NAME);
            HWND hCategoryLabel = GetDlgItem(hDlg, IDC_STATIC_CATEGORY);
            HWND hStockLabel = GetDlgItem(hDlg, IDC_STATIC_STOCK);
            
            if (hNameLabel) SetWindowText(hNameLabel, L"📦 Product Name:");
            if (hCategoryLabel) SetWindowText(hCategoryLabel, L"🏷️ Category:");
            if (hStockLabel) SetWindowText(hStockLabel, L"📊 Quantity:");
            
            // Apply theme to dialog buttons
            HWND hOkBtn = GetDlgItem(hDlg, IDOK);
            HWND hCancelBtn = GetDlgItem(hDlg, IDCANCEL);
            
            if (hOkBtn) {
                SetWindowText(hOkBtn, g_editIndex >= 0 ? L"💾 Update" : L"➕ Add");
                ApplyThemeToButton(hOkBtn, BUTTON_TYPE_PRIMARY, &g_theme);
            }
            if (hCancelBtn) {
                SetWindowText(hCancelBtn, L"❌ Cancel");
                ApplyThemeToButton(hCancelBtn, BUTTON_TYPE_SECONDARY, &g_theme);
            }
            
            // If in edit mode, fill existing values
            if (g_editIndex >= 0 && g_stockManager != NULL)
            {
                StockItem* item = &g_stockManager->items[g_editIndex];
                
                wchar_t wstock[32];
                swprintf(wstock, 32, L"%d", item->stock);
                
                // Convert UTF-8 to wide strings
                wchar_t wname[MAX_NAME_LENGTH];
                wchar_t wcategory[MAX_CATEGORY_LENGTH];
                
                MultiByteToWideChar(CP_UTF8, 0, item->name, -1, wname, MAX_NAME_LENGTH);
                MultiByteToWideChar(CP_UTF8, 0, item->category, -1, wcategory, MAX_CATEGORY_LENGTH);
                
                SetDlgItemText(hDlg, IDC_EDIT_NAME, wname);
                SetDlgItemText(hDlg, IDC_EDIT_CATEGORY, wcategory);
                SetDlgItemText(hDlg, IDC_EDIT_STOCK, wstock);
            }
            
            return TRUE;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    if (g_stockManager == NULL)
                    {
                        EndDialog(hDlg, IDCANCEL);
                        return TRUE;
                    }
                    
                    // Get form data
                    wchar_t wname[MAX_NAME_LENGTH];
                    wchar_t wcategory[MAX_CATEGORY_LENGTH];
                    wchar_t wstockStr[32];
                    
                    GetDlgItemText(hDlg, IDC_EDIT_NAME, wname, MAX_NAME_LENGTH);
                    GetDlgItemText(hDlg, IDC_EDIT_CATEGORY, wcategory, MAX_CATEGORY_LENGTH);
                    GetDlgItemText(hDlg, IDC_EDIT_STOCK, wstockStr, 32);
                    
                    // Convert wide strings to UTF-8
                    char name[MAX_NAME_LENGTH];
                    char category[MAX_CATEGORY_LENGTH];
                    char stockStr[32];
                    
                    WideCharToMultiByte(CP_UTF8, 0, wname, -1, name, MAX_NAME_LENGTH, NULL, NULL);
                    WideCharToMultiByte(CP_UTF8, 0, wcategory, -1, category, MAX_CATEGORY_LENGTH, NULL, NULL);
                    WideCharToMultiByte(CP_UTF8, 0, wstockStr, -1, stockStr, 32, NULL, NULL);
                    
                    int stock = atoi(stockStr);
                    
                    // Validation
                    if (strlen(name) == 0)
                    {
                        ThemedMessageBox(hDlg, L"❌ Product name cannot be empty!", L"Error", MB_OK | MB_ICONERROR);
                        return TRUE;
                    }
                    
                    if (stock < 0)
                    {
                        ThemedMessageBox(hDlg, L"❌ Quantity cannot be negative!", L"Error", MB_OK | MB_ICONERROR);
                        return TRUE;
                    }
                    
                    // Add or update product
                    if (g_editIndex >= 0)
                    {
                        // Update
                        if (UpdateStockItem(g_stockManager, g_editIndex, name, category, stock))
                        {
                            EndDialog(hDlg, IDOK);
                        }
                        else
                        {
                            ThemedMessageBox(hDlg, L"❌ Error updating product!", L"Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    else
                    {
                        // Add
                        if (AddStockItem(g_stockManager, name, category, stock))
                        {
                            EndDialog(hDlg, IDOK);
                        }
                        else
                        {
                            ThemedMessageBox(hDlg, L"❌ Error adding product!", L"Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    
                    return TRUE;
                }
                
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    
    return FALSE;
}
