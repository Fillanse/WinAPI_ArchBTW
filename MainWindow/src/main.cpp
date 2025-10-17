#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "resource.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

const INT BUFFER_SIZE = 256;

const INT BUTTON_WIDTH = 50;
const INT BUTTON_HEIGHT = 40;
const INT BUTTON_SPACING = 8;
const INT MARGIN = 10;

const INT BUTTONS_PER_ROW = 5;
const INT TOTAL_ROWS = 6;

const INT EDIT_HEIGHT = BUTTON_HEIGHT * 0.75;
const INT EDIT_WIDTH = (BUTTON_WIDTH * BUTTONS_PER_ROW) + (BUTTON_SPACING * (BUTTONS_PER_ROW - 1));
const INT WINDOW_WIDTH = (2 * MARGIN) + EDIT_WIDTH + 8;
const INT WINDOW_HEIGHT = (2 * MARGIN) + (2 * EDIT_HEIGHT) + (3 * BUTTON_SPACING) + (TOTAL_ROWS * BUTTON_HEIGHT) + (TOTAL_ROWS * BUTTON_SPACING) + 16;

static WCHAR inputBuffer[BUFFER_SIZE];
static FLOAT memory = 0;
static FLOAT leftOperand;
static FLOAT rightOperand;
static BOOL newInput = TRUE;
static FLOAT lastResult = 0;
static BOOL hasLastResult = FALSE;

FLOAT Division(FLOAT left, FLOAT right) { return left / right; }
FLOAT Multiplication(FLOAT left, FLOAT right) { return left * right; }
FLOAT Subtraction(FLOAT left, FLOAT right) { return left - right; }
FLOAT Addition(FLOAT left, FLOAT right) { return left + right; }
FLOAT Percent(FLOAT value) { return value / 100; }
FLOAT Fraction(FLOAT value) { return value != 0 ? 1 / value : 0; }
FLOAT Square(FLOAT value) { return value * value; }
FLOAT SquareRoot(FLOAT value) { return sqrt(value); }
FLOAT Negate(FLOAT value) { return -value; }

WCHAR GetOperation(HWND hEditOutput)
{
    GetWindowTextW(hEditOutput, inputBuffer, BUFFER_SIZE);
    const WCHAR *operators = L"/*-+";
    WCHAR *opPtr = wcspbrk(inputBuffer, operators);

    if (opPtr != NULL)
    {
        WCHAR op_char = *opPtr;
        *opPtr = L'\0';
        leftOperand = _wtof(inputBuffer);
        rightOperand = _wtof(opPtr + 1);
        *opPtr = op_char;
        return op_char;
    }
    return L'\0';
}

void UpdateResult(HWND hwnd, HWND hEditOutput, WCHAR symbol)
{
    if (newInput) {
        if ((symbol == L'+' || symbol == L'-' || symbol == L'*' || symbol == L'/') && hasLastResult) {
            _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", lastResult);
            SetWindowTextW(hEditOutput, inputBuffer);
        }
        SetWindowTextW(hEditOutput, L"");
        newInput = FALSE;
    }
    
    GetWindowTextW(hEditOutput, inputBuffer, BUFFER_SIZE);
    INT len = lstrlenW(inputBuffer);
    if (len < BUFFER_SIZE - 1)
    {
        inputBuffer[len] = symbol;
        inputBuffer[len + 1] = L'\0';
        SetWindowTextW(hEditOutput, inputBuffer);
    }
}

void UpdateHistory(HWND hwnd, HWND hEditHistory, LPCWSTR expression)
{
    SetWindowTextW(hEditHistory, expression);
}

void ClearAll(HWND hEditResult, HWND hEditHistory)
{
    SetWindowTextW(hEditResult, L"");
    SetWindowTextW(hEditHistory, L"");
    leftOperand = 0;
    rightOperand = 0;
    inputBuffer[0] = L'\0';
    newInput = TRUE;
    hasLastResult = FALSE;
    lastResult = 0;
}

void PerformOperation(HWND hwnd, HWND hEditResult, HWND hEditHistory, WCHAR op)
{
    GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
    
    if (newInput && (op == L'+' || op == L'-' || op == L'*' || op == L'/') && hasLastResult) {
        leftOperand = lastResult;
        WCHAR newBuffer[BUFFER_SIZE];
        _snwprintf(newBuffer, BUFFER_SIZE, L"%g %c ", lastResult, op);
        SetWindowTextW(hEditResult, newBuffer);
        newInput = FALSE;
        return;
    }
    
    const WCHAR *operators = L"/*-+";
    WCHAR *opPtr = wcspbrk(inputBuffer, operators);
    
    if (opPtr != NULL) {
        WCHAR existingOp = *opPtr;
        
        *opPtr = L'\0';
        leftOperand = _wtof(inputBuffer);
        rightOperand = _wtof(opPtr + 1);
        *opPtr = existingOp;
        
        FLOAT result = 0;
        WCHAR history[BUFFER_SIZE];
        _snwprintf(history, BUFFER_SIZE, L"%g %c %g", leftOperand, existingOp, rightOperand);
        
        switch (existingOp) {
        case L'/':
            if (rightOperand == 0) {
                MessageBoxW(hwnd, L"Division by zero", L"Error", MB_ICONERROR);
                result = 0;
            } else {
                result = Division(leftOperand, rightOperand);
            }
            break;
        case L'*':
            result = Multiplication(leftOperand, rightOperand);
            break;
        case L'-':
            result = Subtraction(leftOperand, rightOperand);
            break;
        case L'+':
            result = Addition(leftOperand, rightOperand);
            break;
        }
        
        UpdateHistory(hwnd, hEditHistory, history);
        _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", result);
        SetWindowTextW(hEditResult, inputBuffer);
        
        lastResult = result;
        hasLastResult = TRUE;
        newInput = TRUE;
    } else {
        UpdateResult(hwnd, hEditResult, op);
    }
}

void CreateControls(HWND hwnd)
{
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
                  MARGIN, MARGIN, EDIT_WIDTH, EDIT_HEIGHT,
                  hwnd, (HMENU)IDC_RESULT, NULL, NULL);

    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
                  MARGIN, MARGIN + EDIT_HEIGHT + BUTTON_SPACING,
                  EDIT_WIDTH, EDIT_HEIGHT,
                  hwnd, (HMENU)IDC_HISTORY, NULL, NULL);

    INT baseY = MARGIN + (2 * EDIT_HEIGHT) + (2 * BUTTON_SPACING);

    LPCWSTR memLabels[] = {L"MC", L"MR", L"MS", L"M+", L"M-"};
    INT memIDs[] = {IDC_MC, IDC_MR, IDC_MS, IDC_MPLUS, IDC_MMINUS};

    for (INT i = 0; i < 5; i++)
    {
        CreateWindowW(L"BUTTON", memLabels[i],
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      MARGIN + i * (BUTTON_WIDTH + BUTTON_SPACING), baseY,
                      BUTTON_WIDTH, BUTTON_HEIGHT,
                      hwnd, (HMENU)memIDs[i], NULL, NULL);
    }

    struct Button
    {
        LPCWSTR text;
        INT id;
    } buttons[5][5] = {
        {{L"<-", IDC_BACKSPACE},
         {L"CE", IDC_CE},
         {L"C", IDC_C},
         {L"±", IDC_REVERSE},
         {L"√", IDC_SQRT}},

        {{L"7", IDC_SEVEN},
         {L"8", IDC_EIGHT},
         {L"9", IDC_NINE},
         {L"/", IDC_DIVISION},
         {L"%", IDC_PERCENT}},

        {{L"4", IDC_FOUR},
         {L"5", IDC_FIVE},
         {L"6", IDC_SIX},
         {L"*", IDC_MULTIPLICATION},
         {L"1/x", IDC_FRACTION}},

        {{L"1", IDC_ONE},
         {L"2", IDC_TWO},
         {L"3", IDC_THREE},
         {L"-", IDC_SUBSTRACTION},
         {L"=", IDC_EQUALS}},

        {{L"0", IDC_ZERO},
         {L"", 0},
         {L".", IDC_DOT},
         {L"+", IDC_ADDITION},
         {L"", 0}}};

    for (INT row = 0; row < 5; row++)
    {
        for (INT col = 0; col < 5; col++)
        {
            if (buttons[row][col].text[0] == 0)
                continue;

            INT x = MARGIN + col * (BUTTON_WIDTH + BUTTON_SPACING);
            INT y = baseY + (BUTTON_HEIGHT + BUTTON_SPACING) * (row + 1);
            INT width = BUTTON_WIDTH;
            INT height = BUTTON_HEIGHT;

            if (row == 4 && col == 0)
                width = BUTTON_WIDTH * 2 + BUTTON_SPACING;
            if (row == 3 && col == 4)
                height = BUTTON_HEIGHT * 2 + BUTTON_SPACING;
            if (row == 4 && col == 4)
                continue;

            CreateWindowW(L"BUTTON", buttons[row][col].text,
                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                          x, y, width, height,
                          hwnd, (HMENU)buttons[row][col].id, NULL, NULL);
        }
    }
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, PWSTR lpCmdLine, INT nCmdShow)
{
    const WCHAR CLASS_NAME[] = L"CalculatorWindowClass";

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(
        CLASS_NAME,
        L"Calculator",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (INT)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hEditResult, hEditHistory;

    switch (uMsg)
    {
    case WM_CREATE:
        CreateControls(hwnd);
        hEditResult = GetDlgItem(hwnd, IDC_RESULT);
        hEditHistory = GetDlgItem(hwnd, IDC_HISTORY);
        SetWindowTextW(hEditResult, L"0");
        SetWindowTextW(hEditHistory, L"");
        inputBuffer[0] = L'\0';
        newInput = TRUE;
        hasLastResult = FALSE;
        lastResult = 0;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_ZERO: UpdateResult(hwnd, hEditResult, L'0'); break;
        case IDC_ONE: UpdateResult(hwnd, hEditResult, L'1'); break;
        case IDC_TWO: UpdateResult(hwnd, hEditResult, L'2'); break;
        case IDC_THREE: UpdateResult(hwnd, hEditResult, L'3'); break;
        case IDC_FOUR: UpdateResult(hwnd, hEditResult, L'4'); break;
        case IDC_FIVE: UpdateResult(hwnd, hEditResult, L'5'); break;
        case IDC_SIX: UpdateResult(hwnd, hEditResult, L'6'); break;
        case IDC_SEVEN: UpdateResult(hwnd, hEditResult, L'7'); break;
        case IDC_EIGHT: UpdateResult(hwnd, hEditResult, L'8'); break;
        case IDC_NINE: UpdateResult(hwnd, hEditResult, L'9'); break;
        
        case IDC_DOT: 
            if (wcschr(inputBuffer, L'.') == NULL) {
                UpdateResult(hwnd, hEditResult, L'.');
            }
            break;
        case IDC_DIVISION: 
            PerformOperation(hwnd, hEditResult, hEditHistory, L'/');
            break;
        case IDC_MULTIPLICATION: 
            PerformOperation(hwnd, hEditResult, hEditHistory, L'*');
            break;
        case IDC_SUBSTRACTION: 
            PerformOperation(hwnd, hEditResult, hEditHistory, L'-');
            break;
        case IDC_ADDITION: 
            PerformOperation(hwnd, hEditResult, hEditHistory, L'+');
            break;
        
        case IDC_CE:
            SetWindowTextW(hEditResult, L"0");
            newInput = TRUE;
            break;
        case IDC_C:
            ClearAll(hEditResult, hEditHistory);
            SetWindowTextW(hEditResult, L"0");
            break;
        
        case IDC_BACKSPACE: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            INT len = lstrlenW(inputBuffer);
            if (len > 0) {
                inputBuffer[len - 1] = L'\0';
                if (len == 1) {
                    SetWindowTextW(hEditResult, L"0");
                    newInput = TRUE;
                } else {
                    SetWindowTextW(hEditResult, inputBuffer);
                }
            }
            break;
        }
        
        case IDC_REVERSE: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            FLOAT value = _wtof(inputBuffer);
            value = Negate(value);
            _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", value);
            SetWindowTextW(hEditResult, inputBuffer);
            hasLastResult = TRUE;
            lastResult = value;
            newInput = TRUE;
            break;
        }
        case IDC_SQRT: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            FLOAT value = _wtof(inputBuffer);
            if (value < 0) {
                MessageBoxW(hwnd, L"Invalid input for square root", L"Error", MB_ICONERROR);
            } else {
                value = SquareRoot(value);
                UpdateHistory(hwnd, hEditHistory, L"sqrt");
                _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", value);
                SetWindowTextW(hEditResult, inputBuffer);
                hasLastResult = TRUE;
                lastResult = value;
            }
            newInput = TRUE;
            break;
        }
        case IDC_FRACTION: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            FLOAT value = _wtof(inputBuffer);
            if (value == 0) {
                MessageBoxW(hwnd, L"Division by zero", L"Error", MB_ICONERROR);
            } else {
                value = Fraction(value);
                UpdateHistory(hwnd, hEditHistory, L"1/x");
                _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", value);
                SetWindowTextW(hEditResult, inputBuffer);
                hasLastResult = TRUE;
                lastResult = value;
            }
            newInput = TRUE;
            break;
        }
        
        case IDC_PERCENT: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            FLOAT value = _wtof(inputBuffer);
            value = Percent(value);
            _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", value);
            SetWindowTextW(hEditResult, inputBuffer);
            hasLastResult = TRUE;
            lastResult = value;
            newInput = TRUE;
            break;
        }
        
        case IDC_MS: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            memory = _wtof(inputBuffer);
            break;
        }
        case IDC_MR: {
            _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", memory);
            SetWindowTextW(hEditResult, inputBuffer);
            newInput = TRUE;
            break;
        }
        case IDC_MC: {
            memory = 0;
            break;
        }
        case IDC_MPLUS: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            memory += _wtof(inputBuffer);
            break;
        }
        case IDC_MMINUS: {
            GetWindowTextW(hEditResult, inputBuffer, BUFFER_SIZE);
            memory -= _wtof(inputBuffer);
            break;
        }
        
        case IDC_EQUALS: {
            WCHAR op = GetOperation(hEditResult);
            if (op != L'\0') {
                FLOAT result = 0;
                WCHAR history[BUFFER_SIZE];
                _snwprintf(history, BUFFER_SIZE, L"%g %c %g", leftOperand, op, rightOperand);
                
                switch (op) {
                case L'/':
                    if (rightOperand == 0) {
                        MessageBoxW(hwnd, L"Division by zero", L"Error", MB_ICONERROR);
                        result = 0;
                    } else {
                        result = Division(leftOperand, rightOperand);
                    }
                    break;
                case L'*':
                    result = Multiplication(leftOperand, rightOperand);
                    break;
                case L'-':
                    result = Subtraction(leftOperand, rightOperand);
                    break;
                case L'+':
                    result = Addition(leftOperand, rightOperand);
                    break;
                }
                
                UpdateHistory(hwnd, hEditHistory, history);
                _snwprintf(inputBuffer, BUFFER_SIZE, L"%g", result);
                SetWindowTextW(hEditResult, inputBuffer);
                
                lastResult = result;
                hasLastResult = TRUE;
                newInput = TRUE;
            }
            break;
        }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}