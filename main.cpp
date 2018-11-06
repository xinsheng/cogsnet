//
//  project.cpp
//
//
//  modify loop
//
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iterator>
#include <algorithm>
#include <math.h>
#include <utility>
#include <set>
#include <map>
#include <list>
#define Mu 0.2
#define Life  864000.0
#define Theta Mu/10
#define Lprec 6.0
#define Prec 0.1

//configure
#define TOP_COUNT 4

using namespace std;
inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

void split(string& s, char delim, vector<string>& v) {
	s = trim(s);
	auto i = 0;
	auto pos = s.find(delim);
	while (pos != string::npos) {
		v.push_back(s.substr(i, pos - i));
		i = ++pos;
		pos = s.find(delim, pos);

		if (pos == string::npos)
			v.push_back(s.substr(i, s.length()));
	}
}

void modifyvector(vector<pair<string, double> > & topsignals, pair<string, double> signal) {
	if (topsignals.size() == 0) topsignals.push_back(signal);
	else if (topsignals.size() == 20 && topsignals[19]>signal) {
	}
	else {
		bool insert_success = false;
		for (vector<pair<string, double> >::iterator ptr = topsignals.begin(); ptr<topsignals.end(); ptr++) {
			if (ptr->second < signal.second) {
				topsignals.insert(ptr, signal);
				insert_success = true;
				break;
			}
		}
		if (!insert_success) {
			topsignals.push_back(signal);
		}
		if (topsignals.size()>20) {
			topsignals.erase(topsignals.end() - 1);
		}
	}
}

double absolute(double number) {
	if (number<0) number = -number;
	return number;
}

int findindex(double mu_, int life_, double theta_) {
	int ans = 0;
	ans += (mu_ - 0.02) / 0.001 * 66;
	ans += (life_ - 864000) / 864000 * 11;
	ans += theta_ / 0.002;
	return ans;
}

vector<int> filtertimes(vector <string> *vec, string endtime) {
	vector<int> ret;
	/*
	cout << "count:"<<vec->size()<<endl;
	if(vec->size() > 8000){
		int a =0;
	}*/
	int iendtime = stoi(endtime);
	//time value start at index 3
	for (int i = 3; i < vec->size(); i++) {
		if ((*vec)[i].length()>0) {
			int tmp = stoi((*vec)[i]);
			if (tmp < iendtime) {
				ret.push_back(tmp);
			}
			else {
				break;
			}
		}
	}
	//cout << "]"<<endl;
	return ret;
}

struct IDSignalObj {
	string id;
	double signal;
};

bool signalcompare(const IDSignalObj &a, const IDSignalObj &b) {
	return a.signal > b.signal;
}
//get top 20 signal partners list
list<IDSignalObj> get_top_20(list<IDSignalObj> a) {
	list<IDSignalObj> ret;
	if (a.size()<TOP_COUNT) {
		return a;
	}
	else {
		int i = 0;
		for (list<IDSignalObj>::iterator tmp=a.begin(); tmp != a.end(); tmp++) {
			if (i == TOP_COUNT) break;
			ret.push_back(*tmp);
			i++;
		}
	}
	return ret;
}

map<string, vector<string>> partner_info;
ofstream output_file;

void process(string last_main_id, map<string, string> semester_endtime_map){
	cout << "----process id:"<< last_main_id << endl;
	output_file<< "----process id:"<< last_main_id << endl;
	double mustart = Mu * Prec;
	double mustop = Mu;
	double llimit = Life * Lprec + 1;
	double thetastep = Theta * Prec;
	double thetastop = Theta;
	int loop = 1;

	for (double mu = mustart; mu <= 0.031; mu = mu + Prec / 100) {
		for (double life = Life; life<llimit; life = life + Life) {
			for (double theta = 0.0; theta <= Theta + thetastep / 2; theta += thetastep) {
				cout << "loop:"<<loop++<<endl;
	/*		
	for (double mu = 0; mu < 1; mu++) {
		for (double life = 0; life <1; life++) {
			for (double theta = 0.0; theta < 1; theta++) {
	*/
				map<string,list<IDSignalObj> > semester_partner_signal_map; //semester, top signal partners list
				//foreach all semesters
				for (std::map<string, string>::iterator it = semester_endtime_map.begin(); it != semester_endtime_map.end(); ++it) {
					string current_semester = it->first;
					string endtime = it->second;
					//caculate semester-2/3/5/6 top signals partner
					list<IDSignalObj> partner_signal_list;
					//for (int i = 0; i < myvec.size(); i++) {
					//foreach partner id
					for (map<string, vector<string>>::iterator it1 = partner_info.begin(); it1 != partner_info.end(); ++it1) {
						string partnerid = it1->first;
						vector<int> filter = filtertimes(&it1->second, endtime);
						//caculate this signal
						double signal = mu;
						if (filter.size() > 0) {
							for (int j = 0; j < filter.size() - 1; j++) {
								//fomula
								int dt = filter[j + 1] - filter[j];
								signal = signal * pow(2, -dt / life);
								if (signal < theta) {
									signal = mu;
								}
								else {
									signal = mu + signal * (1 - mu);
								}
							}
						}
						//the final signal
						IDSignalObj obj;
						obj.id = partnerid;
						obj.signal = signal;
						partner_signal_list.push_back(obj);
					}
					//sort by signal
					partner_signal_list.sort(signalcompare);
					//get top 20
					list<IDSignalObj> top20 = get_top_20(partner_signal_list);
					semester_partner_signal_map[current_semester] = top20;;
				}
				for(map<string,list<IDSignalObj> >::iterator  it = semester_partner_signal_map.begin(); it!=semester_partner_signal_map.end(); it++){
					output_file<<"top:"<<it->first<<endl;
					output_file<<"\t";
					for(list<IDSignalObj>::iterator iter = it->second.begin(); iter != it->second.end(); iter++){
						output_file<<iter->id<<"="<<iter->signal<<"\t";
					}
					output_file<<endl;
				}
			}
		}
	}
}

int main() {
	//main_id, <semester,endtime>
	map<string, map<string, string> > main_endtime_map;

	int a = 0, b = 0, c = 0, d = 0;
	cout << "build data 1 struct..." <<endl;
	ifstream infile1;
	infile1.open("survery-short-out.txt");
    std::string tmpline; 
    while (std::getline(infile1, tmpline))
    {
		vector<string> tmpvec;
		split(tmpline, ' ', tmpvec);
		main_endtime_map[tmpvec[0]][tmpvec[7]] = tmpvec[5];
    }
	infile1.close();
	/*
	for(map<string, map<string, string> >::iterator it=main_endtime_map.begin(); it!=main_endtime_map.end(); it++){
		cout << it->first<<endl;
	}*/
	std::cout << "build data 2 struct..." <<endl;
	ifstream infile2;
	infile2.open("cogsnet-mod-count-corr-short.txt");
	//main_id, <partner_id, linedata>
	map<string, map<string, vector<string>> > myvec;//total data
	string last_main_id = "";
	{
		std::getline(infile2, tmpline);
		vector<string> tmpvec;
		split(tmpline, ' ', tmpvec);
		last_main_id = tmpvec[0];
		string partner_id = tmpvec[1];
		partner_info[partner_id] = tmpvec;
	}
	//infile2.close();

	ifstream infile3;
	infile3.open("survey-in.txt");
	std::cout << "processing..." <<endl;
	output_file.open("output.txt");

	//string line;
	//getline(infile2, line);
	//string oldid;
	//infile1 >> oldid;

	vector<pair<int, double> > Jaccard(726, make_pair(0, 0.0));
	map<string, vector<pair<int, double> > > Jaccardmap;
	Jaccardmap["2"] = Jaccard;
	Jaccardmap["3"] = Jaccard;
	Jaccardmap["5"] = Jaccard;
	Jaccardmap["6"] = Jaccard;

	//for(map<string, map<string, vector<string>> >::iterator it=myvec.begin(); it!=myvec.end(); it++)

    while (std::getline(infile2, tmpline))
    {
		vector<string> tmpvec;
		split(tmpline, ' ', tmpvec);
		string main_id = tmpvec[0];
		string partner_id = tmpvec[1];
		if((main_id.compare(last_main_id) != 0)){
			//semester, endtime
			process(last_main_id, main_endtime_map[last_main_id]);
			last_main_id = main_id;
			partner_info.clear();
			partner_info[partner_id] = tmpvec;
		} else {
			partner_info[partner_id] = tmpvec;
		}
	}//main loop
	//process last one
	process(last_main_id, main_endtime_map[last_main_id]);
	/*
	for (map<string, vector<pair<int, double> > >::const_iterator it = Jaccardmap.begin();
		it != Jaccardmap.end(); ++it) {
		myfile1 << "survey number    " << it->first << endl;
		for (int i = 0; i< it->second.size(); i++) {
			myfile1 << it->second[i].first << ' ' << it->second[i].second << endl;
		}
	}*/
	output_file.close();
	cout << "complete" <<endl;
}
