#ifndef THEME_MAKE
#define THEME_MAKE


#ifdef __cplusplus
extern "C" {
#endif
int __Theme_Make(char *ThemeFile, u32 count);
const char *Theme_Title_Name(int);
const char *Theme_Name(int);
int _Extract_Csm();
#ifdef __cplusplus
}
#endif

#endif