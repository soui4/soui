
class CDebug
{
public:
	CDebug(){};
	~CDebug(){};

	static void Debug(pugi::xml_node xmlNode)
	{
		pugi::xml_writer_buff writer;
		xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		SStringW strDebug(writer.buffer(),writer.size());
		SMessageBox(0, S_CW2T(strDebug), _T("提示"), MB_OK);
	};

	static void Debug(SStringT str)
	{
	    SMessageBox(0, str, _T("提示"), MB_OK);
	};
	static SStringT Debug1(pugi::xml_node xmlNode)
	{
		pugi::xml_writer_buff writer;
		xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		return S_CW2T(SStringW(writer.buffer(),writer.size()));
	};

private:

};