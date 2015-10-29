// CheckMethod.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <sstream>
using namespace std;
#define TOTAL_STOCK 16
string stockCode[TOTAL_STOCK] = { "601009", "000001", "002142", "601166", "601939", "601288", "600015", "601988", "600000", "601169", "601998", "600016", "600036", "601328", "601818", "601398" };


string toLower(string s){
	for (int i = 0; i < s.size(); ++i) s[i] = tolower(s[i]);
	return s;
}
void split(char *s, vector<string> &v){
	int len = strlen(s);
	while (len > 0 && isspace(s[len - 1])) s[--len] = 0;
	v.clear();
	string ss(s);
	int last = -1;
	for (int i = 0; i < len; ++i) if (s[i] == ','){
		string tmp;
		tmp = toLower(ss.substr(last + 1, i - last - 1));
		last = i;
		v.push_back(tmp);
	}
	v.push_back(toLower(ss.substr(last + 1, len - last - 1)));
}

string getDigit(string s){
	string result = "";
	for (int i = 0; i < s.size(); ++i)if (isdigit(s[i])){
		result += s[i];
	}
	return result;
}

string getAlpha(string s){
	string result = "";
	for (int i = 0; i < s.size(); ++i)if (isalpha(s[i])){
		result += tolower(s[i]);
	}
	return result;
}

void readFormat(vector<string> &format, FILE *file){
	char s[1000];
	fgets(s, 1000, file);
	split(s, format);
	for (int i = 0; i < format.size(); ++i) format[i] = getAlpha(format[i]);
}
struct DailyData{
	string date;
	double open, high, low, close, volume, adjClose;
};

bool readDailyData(const vector<string> &format, FILE *file, DailyData &data){
	char s[1000];
	if (!fgets(s, 1000, file)) return false;
	vector<string> node;
	split(s, node);
	if (node.size() != format.size()) {
		node.resize(format.size(), "");
	}
	for (int i = 0; i < format.size(); ++i){
		istringstream sin(node[i]);
		if (format[i] == "date"){
			data.date = getDigit(node[i]);
		}
		else if (format[i] == "open"){
			sin >> data.open;
		}
		else if (format[i] == "high"){
			sin >> data.high;
		}
		else if (format[i] == "low"){
			sin >> data.low;
		}
		else if (format[i] == "close"){
			sin >> data.close;
		}
		else if (format[i] == "volume"){
			sin >> data.volume;
		}
		else if (format[i] == "adjclose"){
			sin >> data.adjClose;
		}
		else {
			_ASSERT(false);
		}
	}
	return true;
}

vector<DailyData> data[TOTAL_STOCK];
bool cmpDailyDataByDate(const DailyData &d1, const DailyData &d2){
	return d1.date < d2.date;
}
void readFile(const string &code, vector<DailyData> &data){
	FILE *input = fopen((code + ".csv").c_str(), "r");
	vector<string> format;
	readFormat(format, input);
	DailyData tmp;
	while (readDailyData(format, input, tmp)){
		data.push_back(tmp);
	}
	sort(data.begin(), data.end(), cmpDailyDataByDate);
	fclose(input);
}

void Check(void){
	sort(stockCode, stockCode + TOTAL_STOCK);
	for (int i = 0; i < TOTAL_STOCK; ++i){
		readFile(stockCode[i], data[i]);
	}
	FILE *methodFile = fopen("..\\output.txt", "r");
	char dateS[100], typeS[100], codeS[100];
	string nowCode = "";
	double cash = 10000.0;
	double quantity = 0.0;
	while (fscanf(methodFile, "%s%s%s", dateS, typeS, codeS) == 3){
		string date(dateS), type(typeS), code(codeS);
		if (code == "600000" || code == "601398" || code == "600015")continue;
		int index = lower_bound(stockCode, stockCode + TOTAL_STOCK, code) - stockCode;
		_ASSERT(stockCode[index] == code);
		DailyData tmp;
		tmp.date = date;
		int dateIndex = lower_bound(data[index].begin(), data[index].end(), tmp, cmpDailyDataByDate) - data[index].begin();
		_ASSERT(data[index][dateIndex].date == date);
		double price = (data[index][dateIndex].adjClose / data[index][dateIndex].close * data[index][dateIndex].open);
		if (toLower(type) == "buy"){
			_ASSERT(nowCode == "");
			quantity = cash / price;
			cash = 0.0;
			nowCode = code;
		}
		else {
			_ASSERT(nowCode != "");
			cash = quantity * price * 0.998;
			quantity = 0;
			nowCode = "";
		}
	}
	double result = cash;
	if (nowCode != ""){
		int index = lower_bound(stockCode, stockCode + TOTAL_STOCK, nowCode) - stockCode;
		double price = (data[index].back().adjClose / data[index].back().close * data[index].back().open);
		result = price * quantity;
	}
	printf("Result = %.3f\n", result);
	getchar();
	fclose(methodFile);
}

int _tmain(int argc, _TCHAR* argv[])
{
	Check();
	return 0;
}

