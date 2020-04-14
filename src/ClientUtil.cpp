#include"ClientUtil.h"

char* ClientUtil::fl_str(char* cstr) {
    CString wstr = TEXT(cstr);
    const size_t MAX = 128;
    char* str = (char*)malloc(128);
    fl_utf8fromwc(str, MAX, wstr.AllocSysString(), wcslen(wstr.AllocSysString()));
    return str;
}