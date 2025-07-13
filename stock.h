#ifndef STOCK_H
#define STOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// MinGW-w64 UTF-8 support
#ifdef __MINGW32__
#define _WIN32_WINNT 0x0600
#endif

// Maximum values
#define MAX_NAME_LENGTH 256
#define MAX_CATEGORY_LENGTH 128
#define MAX_ITEMS 1000

// Stock item structure
typedef struct {
    char name[MAX_NAME_LENGTH];
    char category[MAX_CATEGORY_LENGTH];
    int stock;
    int id;
} StockItem;

// Stock manager structure
typedef struct {
    StockItem items[MAX_ITEMS];
    int itemCount;
    int nextId;
} StockManager;

// Function prototypes
void InitStockManager(StockManager* manager);
void FreeStockManager(StockManager* manager);
int AddStockItem(StockManager* manager, const char* name, const char* category, int stock);
int RemoveStockItem(StockManager* manager, int index);
int UpdateStockItem(StockManager* manager, int index, const char* name, const char* category, int stock);
int FindStockItem(StockManager* manager, const char* name);
void SortStockItems(StockManager* manager, int sortBy); // 0=name, 1=stock, 2=category
int SaveStockToFile(StockManager* manager, const char* filename);
int LoadStockFromFile(StockManager* manager, const char* filename);
void SearchStockItems(StockManager* manager, const char* searchTerm, StockItem* results, int* resultCount);
int GetLowStockItems(StockManager* manager, int threshold, StockItem* results, int* resultCount);

// Helper functions for dialog operations
INT_PTR CALLBACK AddEditItemDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void ShowAddItemDialog(HWND parent, StockManager* manager);
void ShowEditItemDialog(HWND parent, StockManager* manager, int itemIndex);

// Global variables (for dialog operations)
extern StockManager* g_stockManager;
extern int g_editIndex;
extern HWND g_hMainWindow;

#endif // STOCK_H
