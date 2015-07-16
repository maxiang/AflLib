#include <jni.h>

class AndroidApp
{
public:
	static void init(JNIEnv* env,jobject obj);
	static int getFileSize(const char* fileName);
	static char* readFile(const char* fileName);
	static bool openImage(const char* fileName,void* dest,int width,int height,bool filter);
	static bool getImageSize(const char* fileName,void* size);
	static void getFontSize(int* size,const char* text,int fontSize,int limitWidth,bool mline);
	static void getFontImage(void* dest,int width,int height,const char* text,int fontSize,int color,int bcolor,int limitWidth,bool mline);
	static void getFontSize(int* size,const wchar_t* text,int fontSize,int limitWidth,bool mline);
	static void getFontImage(void* dest,int width,int height,const wchar_t* text,int fontSize,int color,int bcolor,int limitWidth,bool mline);

protected:
	static JNIEnv* m_env;
	static jobject m_obj;
	static jmethodID MID_open;
	static jmethodID MID_getFileSize;
	static jmethodID MID_openImage;
	static jmethodID MID_getImageSize;
	static jmethodID MID_getFontImage;
	static jmethodID MID_getFontSize;
};
