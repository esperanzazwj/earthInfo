#pragma once
#include "../../util/file.hpp"
#include <string>
#include <map>
#include <regex>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "EngineUtil.h"
using namespace std;


//keep a map :(name - text) ,for shaders,parse simple include

class ShaderProsscer {
public:
	void AddShaderFile(string filename) {
		if (!HasShader(filename)) {
			ParseFile(filename);
		}
	}

	string GetShaderCode(string name) {
		if (ShaderCache.find(name) != ShaderCache.end())
			return ShaderCache[name];
		else
			return "";
	}

	bool HasShader(string name) {
		if (ShaderCache.find(name) != ShaderCache.end())
			return true;
		else
			return false;
	}

	void UpdateShaderFile(string name) {
		auto it = ShaderCache.find(name);
		if (it != ShaderCache.end())
			ShaderCache.erase(it);
		AddShaderFile(name);
	}


	string ParseFileText(string filename, string text)
	{

		text = PreProcess(text);

		string mp = R"(#include +("[^"]+"))";
		regex word_regex(mp);

		//get include file list
		vector<string> IncludeList;
		auto words_begin =
			sregex_iterator(text.begin(), text.end(), word_regex);
		auto words_end = sregex_iterator();
		for (auto i = words_begin; i != words_end; ++i) {
			smatch match = *i;
			string match_str = match.str();
			IncludeList.push_back(match_str);
		}


		//replace include to text
		for (auto& x : IncludeList) {
			string name = GetIncludeName(x);
			if (!HasShader(name))
				ParseFile(name);
			text = Replace(text, x, GetShaderCode(name));
		}

		ShaderCache[filename] = text;

		return text;
	}

	string Replace(string text, string include, string context) {
		regex pattern(include);
		string newtext = regex_replace(text, pattern, context);
		return newtext;
	}

private:

	string GetIncludeName(string s) {
		auto start = s.find_first_of("\"");
		auto last = s.find_last_of("\"");
		return s.substr(start + 1, last - start - 1);
	}

	void ParseFile(string filename) {
		string text = slurp_ignoring_comments(filename);
		path_util::Working_Directory_File_Guard wdfg{ filename };

		ParseFileText(filename, text);
	}

	string slurp_ignoring_comments(path_util::Sanitized_Runtime_Path const& filepath)
	{
		auto content = ss::util::slurp(filepath);

		static std::regex comment_re{"//[^\\n]*(?=\\n)"};
		return regex_replace(content, comment_re, "");
	}

	string PreProcess(string& str)
	{
		string res;
		auto ed = (int)str.size();
		bool cmt = false;
		int i = 0;
		for (i = 0; i < ed-1; ++i)
		{
			if (str[i] == '/' && str[i + 1] == '*')
			{
				cmt = true;
				++i;
			}
			else if (str[i] == '*' && str[i + 1] == '/')
			{
				cmt = false;
				++i;
			}
			else if (!cmt)
				res += str[i];
		}
		if (i < ed)
			res += str[i];

		return res;
	}

	map<string, string> ShaderCache;
};

