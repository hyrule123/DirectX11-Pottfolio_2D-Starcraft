#include "pch.h"

#include "CodeGenFunc.h"
#include "MacroFunc.h"

STRKEY ScriptStandardPrefix = "CScript_";
STRKEY ScriptStandardSuffix = ".h";

void CreateScriptCode()
{
	string ScriptPath = "./Project/Script/";

	//Key = strKey의 변수명, Value = 파일명
	unordered_map<string, string> mapScripts;

	std::filesystem::directory_iterator iter(ScriptPath);
	while (std::filesystem::end(iter) != iter)
	{
		const std::filesystem::directory_entry& entry = *iter;

		string ScriptVal = entry.path().filename().string();

		//"CScript_" 와 .h가 붙어있는 파일들만 추가
		if (string::npos != ScriptVal.find(ScriptStandardPrefix) && string::npos != ScriptVal.find(string(ScriptStandardSuffix)))
		{
			string ScriptKey = ScriptVal;


			//뒷부분(.h) 제거
			size_t pos = ScriptKey.find(ScriptStandardSuffix);
			if (string::npos != pos)
				ScriptKey.erase(pos, string::npos);

			//앞부분 제거
			ScriptKey.erase(0, string(ScriptStandardPrefix).size());

			//대문자로 변경
			transform(ScriptKey.begin(), ScriptKey.end(), ScriptKey.begin(), ::toupper);

			mapScripts.insert(make_pair(ScriptKey, ScriptVal));
		}

		++iter;
	}


	//strKey 작성
	string strKeyPath = ScriptPath + "strKey_Script.h";
	std::ofstream strKey(strKeyPath);
	if (false == strKey.is_open())
		return;

	WriteCodeA(strKey, "#pragma once");
	WriteCodeA(strKey);

	WriteCodeA(strKey, "//This Code is Automatically generated by CodeGenerator.exe");
	WriteCodeA(strKey);

	string Codeline = R"(
#include <Engine/define.h>
)";
	WriteCodeA(strKey, Codeline);
	

	WriteCodeA(strKey, "namespace strKey_SCRIPT");
	WriteBracketOpenA(strKey);
	

	
	const string& BaseCode = string(PresetStr::ConstexprInlineConstChar);
	for (const auto& iter : mapScripts)
	{
		string FinalCodeLine = BaseCode + iter.first + " = ";
		FinalCodeLine += "\"";
		string withoutExt = iter.second;
		withoutExt.erase(withoutExt.length() - 2u, string::npos);
		FinalCodeLine += withoutExt + '"';
		FinalCodeLine += ";";
		WriteCodeA(strKey, FinalCodeLine);
	}

	WriteBracketCloseA(strKey);

	strKey.close();


	//CScriptMgrInitializer 작성
	string ScriptMgrPath = ScriptPath + "CScriptMgrInitializer.cpp";
	std::ofstream ScriptMgr(ScriptMgrPath);
	if (false == ScriptMgr.is_open())
		return;

	//기본 include
	string RawLiteral = R"(
#pragma once

#include "pch.h"
#include "CScriptMgrInitializer.h"

#include <Engine/CScriptMgr.h>

)";
	ScriptMgr << RawLiteral;


	//생성된 스크립트 순회돌며 삽입
	WriteCodeA(ScriptMgr, "//Created Scripts");
	for (const auto& iter : mapScripts)
	{
		IncludeFileA(ScriptMgr, iter.second);
	}
	WriteCodeA(ScriptMgr);


	//init()
	WriteCodeA(ScriptMgr, "void CScriptMgrInitializer::init()");
	WriteBracketOpenA(ScriptMgr);

	string classCodeLine = "CScriptMgr* pMgr = CScriptMgr::GetInst();";
	WriteCodeA(ScriptMgr, classCodeLine);
	
	for (const auto& iter : mapScripts)
	{
		WriteBracketOpenA(ScriptMgr);

		classCodeLine = string("string strKey = ") + string("string(strKey_SCRIPT::") + iter.first + string(");");
		WriteCodeA(ScriptMgr, classCodeLine);

		size_t pos = iter.second.find(string(ScriptStandardSuffix));
		string className = iter.second.substr(0, pos);
		classCodeLine = string("CScript* newScript = new ") + className + string("(strKey);");
		WriteCodeA(ScriptMgr, classCodeLine);

		//classCodeLine = string("newScript->SetKey(strKey)");
		//classCodeLine = string("newScript->SetName(\"") + className + string("\");");
		//WriteCodeA(ScriptMgr, classCodeLine);

		classCodeLine = string("pMgr->AddBaseScript(strKey, newScript);");
		WriteCodeA(ScriptMgr, classCodeLine);

		WriteBracketCloseA(ScriptMgr);
	}

	WriteBracketCloseA(ScriptMgr);


	ScriptMgr.close();
}
