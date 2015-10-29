// StockAnalysis.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
using std::string;
using std::vector;
using std::istringstream;
using std::sort;
using std::unique;
struct DailyData{
	string code, date;
	double open, high, low, close, change, volume, money, traded_market_value, market_value, turnover, adjust_price;
	string report_type, report_date;
	double pe_ttm, ps_ttm, pc_ttm, pb;
	int tag;
};

#define TOTAL_STOCK 16
#define MIN_DATE "2006-01-01"

string stockCode[TOTAL_STOCK] = { "601009", "000001", "002142", "601166","601939","601288", "600015", "601988", "600000", "601169", "601998", "600016", "600036", "601328", "601818", "601398"};
vector<DailyData> data[TOTAL_STOCK];

string getFullCode(string s){
	if (s[0] == '6') return "sh" + s;
	return "sz"+ s;
}

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

void readFormat(vector<string> &format, FILE *file){
	char s[1000];
	fgets(s, 1000, file);
	split(s, format);
}

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
		if (node[i] == ""){
			i = i;
		}
		if (format[i] == "code"){
			data.code = node[i];
		}
		else if (format[i] == "date"){
			data.date = node[i];
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
		else if (format[i] == "change"){
			sin >> data.change;
		}
		else if (format[i] == "volume"){
			sin >> data.volume;
		}
		else if (format[i] == "money"){
			sin >> data.money;
		}
		else if (format[i] == "traded_market_value"){
			sin >> data.traded_market_value;
		}
		else if (format[i] == "market_value"){
			sin >> data.market_value;
		}
		else if (format[i] == "turnover"){
			sin >> data.turnover;
		}
		else if (format[i] == "adjust_price"){
			sin >> data.adjust_price;
		}
		else if (format[i] == "report_type"){
			sin >> data.report_type;
		}
		else if (format[i] == "report_date"){
			sin >> data.report_type;
		}
		else if (format[i] == "pe_ttm"){
			data.pe_ttm = 1e30;
			sin >> data.pe_ttm;
			if (data.pe_ttm < 0) data.pe_ttm = 1e30;
		}
		else if (format[i] == "ps_ttm"){
			data.ps_ttm = 1e30;
			sin >> data.ps_ttm;
		}
		else if (format[i] == "pc_ttm"){
			data.pc_ttm = 1e30;
			sin >> data.pc_ttm;
		}
		else if (format[i] == "pb"){
			data.pb = 1e30;
			sin >> data.pb;
		} else {
			_ASSERT(false);
		}

	}
	return true;
}

void readFile(vector<DailyData> &data, string stockCode){
	stockCode = getFullCode(stockCode);
	FILE *file = fopen((stockCode + ".csv").c_str(), "r");
	vector<string> format;
	readFormat(format, file);
	DailyData tmp;
	while (readDailyData(format, file, tmp)){
		data.push_back(tmp);
	}
	fclose(file);
}

bool cmpDataByDate(const DailyData &d1, const DailyData &d2){
	return d1.date < d2.date;
}

struct ProcessedData{
	double truePrice;
	double PE;
	double PB;
	double change;
	int index;
	int tag;
	ProcessedData(){}
	ProcessedData(double truePrice, double change, double PE, double PB, int index) : truePrice(truePrice), PE(PE), PB(PB), change(change), index(index){}
};

bool cmpProcessedDataByPE(const ProcessedData &d1, const ProcessedData d2){
	return d1.PE < d2.PE;
}

bool cmpProcessedDataByPB(const ProcessedData &d1, const ProcessedData d2){
	return d1.PB < d2.PB;
}

bool decideSell(double cost, double nowPrice, int gapDay){
	if (gapDay <= 2 && nowPrice > cost * 1.01) return true;
	if (nowPrice > cost * 1.030) return true;
	if (gapDay > 30 && nowPrice > cost * 0.9) return true;
	if (gapDay > 50) return true;
	return false;
}

ProcessedData getProcessData(DailyData ori, DailyData pre){
	ProcessedData result;
	double rate = ori.open / ori.close;
	result.truePrice = rate * ori.adjust_price * rate;
	result.PE = rate * ori.pe_ttm * rate;
	result.PB = rate * ori.pb * rate;
	result.change = rate * (1.0 + ori.change) - 1.0;
	//result.PE = pre.pe_ttm;
	//result.PB = pre.pb;
	//result.change = pre.change;
	result.index = ori.tag;
	result.tag = 0;
	return result;
}

int compareStockInDay(vector<DailyData> &stock, vector<DailyData> &pre){
	if (stock.size() < 2) return -1;
	vector<ProcessedData> processedData(stock.size());
	for (int i = 0; i < stock.size(); ++i){
		processedData[i] = getProcessData(stock[i], pre[i]);
	}
	sort(processedData.begin(), processedData.end(), cmpProcessedDataByPE);
	for (int i = 0; i < stock.size() / 8; ++i) processedData[i].tag = 1;
	sort(processedData.begin(), processedData.end(), cmpProcessedDataByPE);
	for (int i = 0; i < stock.size() / 8; ++i) processedData[i].tag = 1;
	double bestChange = 1, bestIndex = -1;
	for (int i = 0; i < processedData.size(); ++i){
		if (processedData[i].tag){
			if (processedData[i].change < bestChange){
				bestChange = processedData[i].change;
				bestIndex = processedData[i].index;
			}
		}
	}
	//if (bestChange > 0) bestIndex = -1;
	return bestIndex;
}

string getDigit(string s){
	string result = "";
	for (int i = 0; i < s.size(); ++i)if (isdigit(s[i])){
		result += s[i];
	}
	return result;
}

void runStratagy(){
	FILE *output = fopen("..\\output.txt", "w");
	double cash = 10000.0;
	double cost = 0.0, quantity = 0.0;
	int buyTime = -1;
	string boughtCode = "";
	vector<string> dates;
	for (int i = 0; i < TOTAL_STOCK; ++i){
		sort(data[i].begin(), data[i].end(), cmpDataByDate);
		int index = 0;
		while (index < data[i].size() && data[i][index].date < MIN_DATE) ++index;
		data[i].erase(data[i].begin(), data[i].begin() + index);
	}
	for (int i = 0; i < TOTAL_STOCK; ++i){
		for (int j = 0; j < data[i].size(); ++j){
			dates.push_back(data[i][j].date);
		}
	}
	sort(dates.begin(), dates.end());
	dates.erase(unique(dates.begin(), dates.end()), dates.end());
	vector<int> heads(TOTAL_STOCK, 0);
	for (int i = 0; i < dates.size(); ++i){
		vector<DailyData> tradableStock;
		vector<DailyData> preStock;
		for (int j = 0; j < TOTAL_STOCK; ++j){
			while (heads[j] < data[j].size() && data[j][heads[j]].date < dates[i]) ++heads[j];
			if (heads[j] < data[j].size() && data[j][heads[j]].date == dates[i] && heads[j] > 0){
				tradableStock.push_back(data[j][heads[j]]);
				tradableStock.back().tag = j;
				preStock.push_back(data[j][heads[j] - 1]);
			}
		}
		int bestBuy = compareStockInDay(tradableStock, preStock);
		int sellIndex = -1;
		double total = cash;
		if (boughtCode != ""){
			for (int i = 0; i < tradableStock.size(); ++i) if (tradableStock[i].code == boughtCode){
				sellIndex = tradableStock[i].tag;
				break;
			}
			if (sellIndex != -1){
				ProcessedData processedData = getProcessData(data[sellIndex][heads[sellIndex]], data[sellIndex][heads[sellIndex]]);
				if (decideSell(cost, processedData.truePrice, i - buyTime)){
					fprintf(output, "%s Sell %s\n", getDigit(dates[i]).c_str(), getDigit(boughtCode).c_str());
					cash = processedData.truePrice * quantity * 0.998;
					buyTime = -1;
					boughtCode = "";
					quantity = cost = 0.0;
				}
				total = quantity * processedData.truePrice + cash;
			}
		}
		if (bestBuy != -1 && boughtCode == "" && bestBuy != sellIndex){
			ProcessedData processedData = getProcessData(data[bestBuy][heads[bestBuy]], data[bestBuy][heads[bestBuy]]);
			fprintf(output, "%s Buy %s\n", getDigit(dates[i]).c_str(), getDigit(data[bestBuy][heads[bestBuy]].code).c_str());
			quantity = cash / processedData.truePrice;
			cost = processedData.truePrice;
			buyTime = i;
			boughtCode = data[bestBuy][heads[bestBuy]].code;
			cash = 0.0;
			total = quantity * processedData.truePrice + cash;
		}

		printf("Date %s Cash = %.3f Total = %.3f Has %s Quantity = %.3f Cost = %.3f\n", dates[i].c_str(), cash, total, boughtCode.c_str(), quantity, cost);
		//getchar();
	}
	fclose(output);
}

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i = 0; i < 16; ++i) {
		readFile(data[i], stockCode[i]);
	}
	runStratagy();
	getchar();
	return 0;
}

