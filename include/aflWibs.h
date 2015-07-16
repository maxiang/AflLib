#include "aflCgi.h"
#include "aflSqlite.h"
#include "aflMd5.h"
#include "aflAjax.h"
using namespace AFL;
using namespace AFL::CGI;
using namespace AFL::SQLITE;

class WibsBase
{
public:
	void setStream(Stream* s)
	{
		m_stream = s;
	}
protected:
	WibsBase();
	virtual bool init();
	bool outRes(LPCSTR sql);
	void outRes(SQRes& res);
	void outResHeader(SQRes& res);
	void outResData(SQRes& res);

	bool command();
	void outLogin();
	bool isEdit();
	INT getUserID();
	bool isUser();
	void outLogList();
	void deleteUser();
	void writeUser();
	void outUserList();

	void outFile();
	void deleteFileLink();
	void outFileLink();

	void writeFileLink();
	void deleteMessageLink();
	void writeMessageLink();
	void outMessageLink();

	void outUpload();
	void removeFile();
	void renameFile();
	void makeDir();
	void moveFile();

	bool jump();

	AccessLog m_accessLog;
	SQLite m_sqlite;
	Stream* m_stream;
	INT m_edit;
	LPCSTR m_dataDir;
};
