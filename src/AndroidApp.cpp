#include <string.h>
#include <android/log.h>
#include "aflStd.h"
#include "AndroidApp.h"

using namespace AFL;

JNIEnv* AndroidApp::m_env;
jobject AndroidApp::m_obj;
jmethodID AndroidApp::MID_open;
jmethodID AndroidApp::MID_getFileSize;
jmethodID AndroidApp::MID_openImage;
jmethodID AndroidApp::MID_getImageSize;
jmethodID AndroidApp::MID_getFontSize;
jmethodID AndroidApp::MID_getFontImage;

void AndroidApp::init(JNIEnv* env,jobject obj)
{
	m_env = env;
	m_obj = obj;

	//クラス取得
	jclass jcls = env->GetObjectClass(  obj );
	//メソッド取得
	MID_open = env->GetMethodID(  jcls, "open", "(Ljava/lang/String;)[B");
	MID_getFileSize = env->GetMethodID(  jcls, "getFileSize", "(Ljava/lang/String;)I");
	MID_openImage = env->GetMethodID(  jcls, "openImage", "(Ljava/lang/String;Ljava/nio/ByteBuffer;IIZ)Z");
	MID_getImageSize = env->GetMethodID(  jcls, "getImageSize", "(Ljava/lang/String;)[I");
	MID_getFontSize = env->GetMethodID(  jcls, "getFontSize", "(Ljava/lang/String;IIZ)[I");
	MID_getFontImage = env->GetMethodID(  jcls, "getFontImage", "(Ljava/nio/ByteBuffer;IILjava/lang/String;IIIIZ)Z");

	env->DeleteLocalRef(jcls);

}
char* AndroidApp::readFile(const char* fileName)
{
	//文字列の設定
	jstring strj = m_env->NewStringUTF(fileName);


	//メソッド呼び出し
	jbyteArray arrj = (jbyteArray)m_env->CallObjectMethod(  m_obj, MID_open,strj );

	jboolean bret;
	jbyte* arrSrc=m_env->GetByteArrayElements(arrj,&bret);
	int nSize = m_env->GetArrayLength(arrj);

	char* data = new char[nSize+1];
	memcpy(data,arrSrc,nSize);
	data[nSize] = 0;

	m_env->DeleteLocalRef (strj);
	m_env->ReleaseByteArrayElements (arrj,arrSrc,0);
	return data;
}
int AndroidApp::getFileSize(const char* fileName)
{
	//文字列の設定
	jstring strj = m_env->NewStringUTF(fileName);
	//メソッド呼び出し
	jint size = (jint)m_env->CallIntMethod(  m_obj, MID_getFileSize,strj );
	m_env->DeleteLocalRef (strj);
	return size;
}
void AndroidApp::getFontSize(int* size,const char* text,int fontSize,int limitWidth,bool mline)
{
	getFontSize(size,UCS2(text),fontSize,limitWidth,mline);
}
void AndroidApp::getFontSize(int* size,const wchar_t* text,int fontSize,int limitWidth,bool mline)
{
	//文字列の設定
	jstring strj = m_env->NewString((const jchar*)text,wcslen(text)*2);
	//メソッド呼び出し
	jintArray arrj = (jintArray)m_env->CallObjectMethod(  m_obj, MID_getFontSize,strj,fontSize,limitWidth,mline );
	m_env->DeleteLocalRef (strj);

	if(arrj)
	{
		int  s = m_env->GetArrayLength (arrj);
		__android_log_print( ANDROID_LOG_FATAL,"FONT","test1 %d\n",s);
		jint* arrSrc = (jint*)m_env->GetIntArrayElements(arrj,NULL);
		__android_log_print( ANDROID_LOG_FATAL,"FONT","test2\n");
		size[0] = arrSrc[0];
		size[1] = arrSrc[1];
		m_env->ReleaseIntArrayElements (arrj,arrSrc,0);
	}

}
void AndroidApp::getFontImage(void* dest,int width,int height,const char* text,int fontSize,int color,int bcolor,int limitWidth,bool mline)
{
	getFontImage(dest,width,height,UCS2(text),fontSize,color,bcolor,limitWidth,mline);
}
void AndroidApp::getFontImage(void* dest,int width,int height,const wchar_t* text,int fontSize,int color,int bcolor,int limitWidth,bool mline)
{
	int length = width * height * 4;
	jobject buffer = m_env->NewDirectByteBuffer(dest,length);

	//文字列の設定
	jstring strj = m_env->NewString((const jchar*)text,wcslen(text)*2);
	//メソッド呼び出し
	jintArray result = (jintArray)m_env->CallIntMethod(  m_obj, MID_getFontImage,buffer,width,height,strj,fontSize,color,bcolor,limitWidth,mline );
	m_env->DeleteLocalRef (strj);
}

bool AndroidApp::getImageSize(const char* fileName,void* size)
{
	//文字列の設定
	jstring strj = m_env->NewStringUTF(fileName);
	//メソッド呼び出し
	jintArray arrj = (jintArray)m_env->CallObjectMethod(  m_obj, MID_getImageSize,strj );
	m_env->DeleteLocalRef (strj);
	if(arrj)
	{
		jint* arrSrc = (jint*)m_env->GetIntArrayElements(arrj,NULL);
		((int*)size)[0] = arrSrc[0];
		((int*)size)[1] = arrSrc[1];

		m_env->ReleaseIntArrayElements (arrj,arrSrc,0);
		return true;
	}
	return false;
}
bool AndroidApp::openImage(const char* fileName,void* dest,int width,int height,bool filter)
{
	int size[2];
	if(!getImageSize(fileName,size))
		return false;

	int length = width * height * 4;

	jobject buffer = m_env->NewDirectByteBuffer(dest,length);

	//文字列の設定
	jstring strj = m_env->NewStringUTF(fileName);
	//メソッド呼び出し
	jboolean ret = m_env->CallBooleanMethod(  m_obj, MID_openImage,strj,buffer,width,height,filter);
	m_env->DeleteLocalRef (strj);
	m_env->DeleteLocalRef (buffer);

	return ret;
}
