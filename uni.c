#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, ""); // Enable Unicode support

    for (int i = 32; i < 10000; i++) { // Print from Unicode 32 to 999
        wprintf(L"%4d: %lc   ", i, (wchar_t)i); // Print Unicode code point and character
        if (i % 10 == 0) wprintf(L"\n"); // New line every 10 characters
    }

    return 0;
}