#include "aflWindow.h"
#include "../aflWintool.h"

using namespace AFL::WINDOWS;

class FormText : public Window
{
public:
	FormText(HWND parent,UINT id=0)
	{
		create(parent,id);
	}
	bool create(HWND parent,UINT id=0)
	{
		HDC hdc = GetDC(NULL);
		TEXTMETRIC tm;
		GetTextMetrics( hdc, &tm) ;
		INT height = ( tm.tmHeight + tm.tmExternalLeading) * 7 / 6 ;
		moveWindow(0,0,100,height);
		ReleaseDC(NULL,hdc);

		setStyleEx(WS_EX_CLIENTEDGE);
		if(!createWindow(L"EDIT",L"test",WS_CHILD | WS_VISIBLE|ES_AUTOHSCROLL ,parent,id))
			return false;
		return true;
	}
};

class FormButton : public Window
{
public:
	FormButton()
	{
	}
	FormButton(HWND parent,UINT id=0)
	{
		create(parent,id);
	}
	bool create(HWND parent,UINT id=0)
	{
		HDC hdc = GetDC(NULL);
		TEXTMETRIC tm;
		GetTextMetrics( hdc, &tm) ;
		INT height = ( tm.tmHeight + tm.tmExternalLeading) * 7 / 6 ;
		moveWindow(0,0,100,height);
		ReleaseDC(NULL,hdc);

		if(!createWindow("BUTTON","test",WS_CHILD | WS_VISIBLE|BS_PUSHBUTTON ,parent,id))
			return false;
		return true;
	}
};

class Form
{
	friend class FormManager;
public:
	~Form()
	{
		if(m_window)
			delete m_window;
	}
	bool operator<(LPCSTR name) const
	{
		return m_name < name;
	}
	bool operator==(LPCSTR name) const
	{
		return m_name == name;
	}
	bool operator==(const Form& form) const
	{
		return m_name == form.m_name && m_index == form.m_index;
	}
	bool operator<(const Form& form) const
	{
		if(m_name < form.m_name)
			return true;
		if(m_index < form.m_index)
			return true;
		return false;
	}
	INT getIndex()const{return m_index;}
	Window* getWindow()const{return m_window;}

	bool show(INT cmdShow=SW_SHOW) const
	{
		if(!this || !m_window)
			return false;
		return m_window->showWindow(cmdShow);
	}
	bool position(INT x,INT y) const
	{
		if(!this || !m_window)
			return false;
		m_window->setPos(x,y);
		return true;
	}
protected:
	Form()
	{
		m_index = 0;
		m_window = 0;
		m_class = NULL;
	}
	Form(LPCSTR name,INT index=0)
	{
		m_name = name;
		m_index = index;
		m_window = NULL;
		m_class = NULL;
	}

	void set(Window* window,LPCSTR name,INT index,LPCSTR cs)
	{
		m_window = window;
		m_name = name;
		m_index = index;
		m_class = cs;
	}
	Window* m_window;
	std::string m_name;
	LPCSTR m_class;
	INT m_index;
};

class FormManager
{
public:
	bool createWindow(LPCSTR name,INT index=0,LPCSTR cs=NULL,LPCSTR parentName=NULL,INT parentIndex=0)
	{
		static LPCSTR className[] =
		{
			"FRAME","CHILD","SPLIT","BUTTON","TEXT",NULL
		};
		
		HWND parentWnd = NULL;
		if(parentName)
		{
			Window* parent = NULL;
			parent = getWindow(parentName,parentIndex);
			if(parent)
				parentWnd = parent->getWnd();
		}	
		Window* window = NULL;
		INT classIndex = 0;
		if(cs)
		{
			INT i;
			for(i=0;className[i];++i)
			{
				if(strcmp(cs,className[i]) == 0)
					classIndex = i;
			}
		}
		switch(classIndex)
		{
		case 0: //フレームウインドウ
			window = new Window;
			window->createWindow();
			break;
		case 1: //子ウインドウ
			window = new Window();
			   
			window->createWindow(NULL,NULL,WS_CHILD|WS_VISIBLE|WS_CAPTION |WS_CLIPSIBLINGS,parentWnd);
			window->moveWindow(0,0,100,100);
			break;
		case 3: //ボタン
			window = new FormButton(parentWnd);
			break;
		case 4:
			window = new FormText(parentWnd);
			break;
		}
		if(!window)
			return false;
		if(isForm(name,index))
		{
			index = getLastIndex(name) + 1;
		}
		std::multiset<Form>::iterator it = m_form.insert(Form());
		it->set(window,name,index,className[classIndex]);
		return true;
	}
	bool isForm(LPCSTR name,INT index=0) const
	{
		std::multiset<Form>::const_iterator it = m_form.find(Form(name,index));
		if(it == m_form.end())
			return false;
		return true;
	}
	INT getLastIndex(LPCSTR name) const
	{
		std::multiset<Form>::const_iterator it = m_form.find(Form(name));
		INT index = -1;
		for(;it == m_form.end() && *it == name;++it)
		{
			index = it->getIndex();
		}
		return index;
	}
	Window* getWindow(LPCSTR name,INT index=0) const
	{
		std::multiset<Form>::const_iterator it = m_form.find(Form(name,index));
		if(it == m_form.end())
			return NULL;
		return it->getWindow();
	}
	Form* get(LPCSTR name,INT index=0)
	{
		std::multiset<Form>::iterator it = m_form.find(Form(name,index));
		if(it == m_form.end())
			return NULL;
		return &*it;
	}
protected:
	std::multiset<Form> m_form;
};

