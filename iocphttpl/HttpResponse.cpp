#include "HttpResponse.h"


HttpResponse::HttpResponse()
{
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);
}


HttpResponse::~HttpResponse()
{

	::WaitForSingleObject(ghMutex, INFINITE);
	ibuflist_t itr;
	for (itr = m_bufferList.begin(); itr != m_bufferList.end(); itr++)
	{
		lpstatic_content_t p = *itr;
		if (p != NULL)
		{
			if (p->name	!= NULL)
			{
				free(p->name);
				p->name = NULL;
			}
			if (p->content != NULL)
			{
				free(p->content);
				p->content = NULL;
			}
			//free(p->bytcontent);
			free(p);
			p = NULL;
		}
	}
	::ReleaseMutex(ghMutex);
	::CloseHandle(ghMutex);
}


void HttpResponse::Write(const char* str)
{
	std::vector<byte> rv;
	rv.assign(str, str + strlen(str));
	m_sbResponsePackage = rv;
	//m_szResponse = std::string(str);
}

void HttpResponse::WriteTemplate(char* code)
{
	char buf[DATA_BUFSIZE];
	memset(buf, '\0', DATA_BUFSIZE);
	std::string str = GetTemplateContent(code);
	std::string filename = GetTemplateFilepath(code);
	GetPathExtension(filename.c_str());
	SetContentTypeFromExtension();
	std::vector<byte> rv;
	const char *ret = str.c_str();
	rv.assign(ret, ret + strlen(ret));
	m_sbResponsePackage = rv;
}

void HttpResponse::SetStaticFileName(string path)
{
	m_path = path;
	GetPathExtension(path.c_str());
	SetContentTypeFromExtension();
}

PTSTR  HttpResponse::GetPathExtension(const char* pszPath)
{
	std::string str = std::string(pszPath);
	std::wstring wstr = std::wstring(str.begin(), str.end());
	PTSTR pstr = ::PathFindExtension(wstr.c_str());
	pathExtension.assign(pstr);
	szPath = pszPath;
	return pstr;
}

PTSTR  HttpResponse::GetPathExtension(PTSTR pszPath)
{
	PTSTR str = ::PathFindExtension(pszPath);
	pathExtension.assign(str);
	return str;
}

void HttpResponse::SetContentTypeFromExtension()
{
	contenType.assign(L"text/html");
	std::wstring wstr;
	wstr.assign(pathExtension);
	if (wstr.compare(L".jpg") == 0)
	{
		contenType.assign(L"image/jpg");
	}
	if (wstr.compare(L".png") == 0)
	{
		contenType.assign(L"image/png");
	}
	if (wstr.compare(L".css") == 0)
	{
		contenType.assign(L"text/css");
	}
	if (wstr.compare(L".js") == 0)
	{
		contenType.assign(L"application/javascript"); 
	}
}

void HttpResponse::SetContentType(const char* str)
{
	std::string s;
	s.assign(str);
	contenType.assign(s.begin(), s.end());
}

const char* HttpResponse::GetContent()
{
	return "";
}

std::vector<byte> HttpResponse::GetStaticContent(const char *path)
{
	DWORD dwThreadId = GetCurrentThreadId();
	printf("%d::Reading filename: %s \n", dwThreadId, path);
	::WaitForSingleObject(ghMutex, INFINITE);

	/*
	ibuflist_t itr;
	for (itr = m_bufferList.begin(); itr != m_bufferList.end(); itr++)
	{
		lpstatic_content_t pitem = *itr;
		int diff = strcmp(pitem->name, path);
		if (diff == 0)
		{
			pitem->rawtime = time(0);
			::ReleaseMutex(ghMutex);
			return *pitem->bytcontent;
		}
	}
	*/

	// open the file:
	std::ifstream file(path, std::ios::binary);
	
	// Stop eating new lines in binary mode!!!
	file.unsetf(std::ios::skipws);

	// get its size:
	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	std::vector<BYTE> vec;
	vec.reserve(fileSize);

	// read the data:
	vec.insert(vec.begin(),
		std::istream_iterator<BYTE>(file),
		std::istream_iterator<BYTE>());

	/*
	lpstatic_content_t p1 = (lpstatic_content_t)malloc(sizeof(static_content_t));
	p1->name = _strdup(path);
	p1->bytcontent = new std::vector<byte>();
	p1->bytcontent->assign(vec.begin(), vec.end());
	p1->rawtime = time(0);
	m_bufferList.push_back(p1);
	*/

	::ReleaseMutex(ghMutex);

	return vec;
}

std::vector<byte> HttpResponse::GetStaticContent3(const char *file_name)
{
	// There's a bug here, str will stop at '\0' for binary chars.
	// Won't work for images and other bin files.
	char *buf = GetStaticContent2(file_name);
	std::vector<byte> bytbuf;
	std::string str;
	str.assign(buf);
	bytbuf.assign(str.begin(), str.end());
	return bytbuf;
}

char* HttpResponse::GetStaticContent2(const char *file_name)
{
	::WaitForSingleObject(ghMutex, INFINITE);
	
	/*
	time_t now = time(0);
	ibuflist_t itr;
	for (itr = m_bufferList.begin(); itr != m_bufferList.end(); itr++)
	{
		lpstatic_content_t pitem = *itr;
		int diff = strcmp(pitem->name, file_name);
		if (diff == 0)
		{
			pitem->rawtime = time(0);
			::ReleaseMutex(ghMutex);
			return pitem->content;
		}
		double dt = difftime(now, pitem->rawtime);

	}
	*/

	FILE *fp;
	char *buf = NULL;

	fopen_s(&fp, file_name, "rb");
	if (fp == NULL)
	{
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	buf = (char*)malloc(size + 1);
	memset(buf, 0, size);

	int i = 0;
	int ch;
	while ((ch = fgetc(fp)) != EOF)
	{
		buf[i++] = ch;
	}

	fclose(fp);

	/*
	lpstatic_content_t p1 = (lpstatic_content_t)malloc(sizeof(static_content_t));
	p1->name = _strdup(file_name);
	p1->content = buf;
	p1->rawtime = time(0);
	m_bufferList.push_back(p1);
	*/

	::ReleaseMutex(ghMutex);

	return buf;

}

char* HttpResponse::GetStaticContent(std::wstring wfile_name)
{

	std::string file_name;
	file_name.assign(wfile_name.begin(), wfile_name.end());
	char *buf = GetStaticContent2(file_name.c_str());

	return buf;
}

void HttpResponse::WriteStatic(const char *path)
{
	m_sbResponsePackage = GetStaticContent(path);
	m_pszResponsePackage = GetStaticContent2(path);
}


void HttpResponse::GetResponse(char* pszResponse, vector<byte> *pvb, DWORD dwSize)
{
	DWORD dwThreadId = GetCurrentThreadId();
	ZeroMemory(pszResponse, dwSize);
	sprintf_s(pszResponse, dwSize, "%s%s", resp_ok, "\n");
	sprintf_s(pszResponse, dwSize, "%s%s%s%s", pszResponse, "Date: " , "HTTP/1.x 200 OK" , "\n");
	std::string ctstr;
	ctstr.assign(contenType.begin(), contenType.end());
	sprintf_s(pszResponse, dwSize, "%s%s%s%s", pszResponse, "Content-Type: ", ctstr.c_str(), "\n");

	size_t siz = m_sbResponsePackage.size();
	sprintf_s(pszResponse, dwSize, "%s%s%d%s", pszResponse, "Content-Length: ", siz, "\n\n");
	std::string str;
	str.assign(pszResponse);

	std::string str2(m_sbResponsePackage.begin(), m_sbResponsePackage.end());
	str.insert(str.end(), str2.begin(), str2.end());
	sprintf_s(pszResponse, dwSize, "%s", str.c_str());
	m_sbResponsePackage.assign(str.begin(), str.end());
	pvb->assign(m_sbResponsePackage.begin(), m_sbResponsePackage.end());
}

/*
byte* HttpResponse::GetResponse2(ULONG *len)
{
	*len = 0;

	std::string ctstr;
	ctstr.assign(contenType.begin(), contenType.end());

	std::ostringstream oss;
	oss << resp_ok << "\n";
	oss << "Date: " << "May 10, 2015" << "\n";
	oss << "Content-Type: " << ctstr.c_str() << "\n";
	oss << "Content-Length: " << m_sbResponse.size() << "\n";
	oss << "\n";

	std::string s = oss.str();

	int bufsiz = s.length() + m_sbResponse.size();
	byte* buffer2 = (byte*)malloc(bufsiz);
	if (buffer2)
	{
		std::copy(s.begin(), s.end(), buffer2);
		std::copy(m_sbResponse.begin(), m_sbResponse.end(), &buffer2[s.length()]);
		*len = bufsiz;
	}

	return buffer2;
}
*/

byte* HttpResponse::GetResponse2(ULONG *len)
{
	DWORD dwThreadId = GetCurrentThreadId();
	char *buffer = (char*)malloc(DATA_BUFSIZE);
	memset(buffer, 0, DATA_BUFSIZE);

	std::vector<byte> binbuffer = m_sbResponsePackage;

	std::string ctstr;
	ctstr.assign(contenType.begin(), contenType.end());
	size_t siz = binbuffer.size();
	std::string ssiz = std::to_string(siz);

	strcpy_s(buffer, DATA_BUFSIZE, resp_ok);
	strcat_s(buffer, DATA_BUFSIZE, "\n");
	strcat_s(buffer, DATA_BUFSIZE, "Date: ");
	strcat_s(buffer, DATA_BUFSIZE, "May 10, 2015");
	strcat_s(buffer, DATA_BUFSIZE, "\n");
	strcat_s(buffer, DATA_BUFSIZE, "Content-Type: ");
	strcat_s(buffer, DATA_BUFSIZE, ctstr.c_str());
	strcat_s(buffer, DATA_BUFSIZE, "\n");
	strcat_s(buffer, DATA_BUFSIZE, "Content-Length: ");
	strcat_s(buffer, DATA_BUFSIZE, ssiz.c_str());
	strcat_s(buffer, DATA_BUFSIZE, "\n");
	strcat_s(buffer, DATA_BUFSIZE, "\n");

	int bufsiz = strlen(buffer) + binbuffer.size() + 1;
	byte* buffer2 = (byte*)malloc(bufsiz);
	memset(buffer2, 0, bufsiz);
	strcpy_s((char*)buffer2, strlen(buffer)+1, buffer);
	int n = strlen((char*)buffer2);

	std::vector<byte>::iterator it;
	for (it = binbuffer.begin(); it != binbuffer.end(); it++)
	{
		byte b = (byte)*it;
		buffer2[n++] = b;
	}

	*len = strlen((char*)buffer2);

	return buffer2;
}

void HttpResponse::GetResponse3(std::vector<byte> *pvbHeaderContent)
{
	DWORD dwThreadId = GetCurrentThreadId();

	std::string ctstr;
	ctstr.assign(contenType.begin(), contenType.end());

	std::ostringstream oss;
	oss << resp_ok << "\n";
	oss << "Date: " << "May 10, 2015" << "\n";
	oss << "Content-Type: " << ctstr.c_str() << "\n";
	oss << "Content-Length: " << m_sbResponsePackage.size() << "\n";
	oss << "\n";
	std::string textBuffer = oss.str();

	std::vector<byte> tmp;
	tmp.insert(tmp.end(), textBuffer.begin(), textBuffer.end());
	tmp.insert(tmp.end(), m_sbResponsePackage.begin(), m_sbResponsePackage.end());
	
	pvbHeaderContent->clear();
	pvbHeaderContent->assign(tmp.begin(), tmp.end());

	return;
}
