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

using namespace std;

void split(const string& s, char delim, vector<string>& v) {
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

vector<int> filtertimes(vector <string> vec, string endtime) {
	vector<int> ret;
	int iendtime = stoi(endtime);
	//time value start at index 3
	for (int i = 3; i < vec.size(); i++) {
		if (vec[i].length()>0) {
			int tmp = stoi(vec[i]);
			if (tmp < iendtime) {
				ret.push_back(tmp);
			}
			else {
				break;
			}
		}
	}
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
	if (a.size()<20) {
		return a;
	}
	else {
		int i = 0;
		for (IDSignalObj tmp : a) { 
			if (i == 20) break;
			ret.push_back(tmp);
			i++;
		}
	}
	return ret;
}

int main() {
	int a = 0, b = 0, c = 0, d = 0;
	ifstream infile1;
	infile1.open("survery-short-out.txt");
	ifstream infile2;
	infile2.open("cogsnet-mod-count-corr-short-1.txt");
	ifstream infile3;
	infile3.open("survey-in.txt");

	ofstream myfile1;
	myfile1.open("output.txt");

	string line;
	getline(infile2, line);
	string oldid;
	infile1 >> oldid;

	vector<pair<int, double> > Jaccard(726, make_pair(0, 0.0));
	map<string, vector<pair<int, double> > > Jaccardmap;
	Jaccardmap["2"] = Jaccard;
	Jaccardmap["3"] = Jaccard;
	Jaccardmap["5"] = Jaccard;
	Jaccardmap["6"] = Jaccard;
	while (true) {
		bool end = false;
		vector<vector <string> > myvec;//mod count, only test 10060
		while (true) {
			vector<string> myvec_inside;
			split(line, ' ', myvec_inside);
			myvec.push_back(myvec_inside);
			getline(infile2, line);
			bool moreline = (line.length() > 0);
			if (!moreline) end = true;
			if (end) break;
			vector<string> newvec;
			split(line, ' ', newvec);
			if (newvec[0] != myvec_inside[0]) break;
		}
		map <string, string> surveytime;//semester, endtime
		map <string, int> PartnerNo;//
		while (infile1) {
			string id, time, surveynum, patners;
			id = oldid;
			infile1 >> time >> surveynum >> patners;
			surveytime[surveynum] = time;
			PartnerNo[surveynum] = atof(patners.c_str());
			if (infile1.eof()) break;
			infile1 >> oldid;
			if (oldid != id) break;
		}
		map<string, vector<int> > partnersbysurvey;
		for (map<string, int>::iterator it3 = PartnerNo.begin();
			it3 != PartnerNo.end(); ++it3)
		{
			vector<int> parters2;
			for (int i = 0; i<it3->second; i++) {
				string in1, in2, in3, in4, in5, in6;
				infile3 >> in1 >> in2 >> in3 >> in4 >> in5 >> in6;
				parters2.push_back(atof(in2.c_str()));
			}
			partnersbysurvey[it3->first] = parters2;
		}

		double mustart = Mu * Prec;
		double mustop = Mu;
		double llimit = Life * Lprec + 1;
		double thetastep = Theta * Prec;
		double thetastop = Theta;

		int itotal = 0;
		for (double mu = mustart; mu <= 0.031; mu = mu + Prec / 100) {
			for (double life = Life; life<llimit; life = life + Life) {
				for (double theta = 0.0; theta <= Theta + thetastep / 2; theta += thetastep) {

					map<string,list<IDSignalObj> > semester_partner_signal_map; //semester, top signal partners list
					//foreach all semesters
					for (std::map<string, string>::iterator it = surveytime.begin(); it != surveytime.end(); ++it) {
						string current_semester = it->first;
						string endtime = it->second;
						//caculate semester-2/3/5/6 top signals partner
						list<IDSignalObj> partner_signal_list;
						for (int i = 0; i < myvec.size(); i++) {
							string partnerid = myvec[i][1];
							vector<int> filter = filtertimes(myvec[i], endtime);
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

					std::cout << itotal++ <<":done" << endl;
					/*
					map <string, vector<pair<string, double> > > top4signals;
					vector<pair<string, double> > temp;
					for (map<string, string>::const_iterator it = surveytime.begin();
						it != surveytime.end(); ++it)
					{
						top4signals[it->first] = temp;
					}
					for (int i = 0; i<myvec.size(); i++) {
						double signal = 0;
						int nums = atof(myvec[i][2].c_str());
						map<string, pair<string, double> > signal4;
						for (int j = 4; j < nums + 3; j++) {
							for (map<string, string>::const_iterator it = surveytime.begin();
								it != surveytime.end(); ++it)
							{
								if (signal4.find(it->first) == signal4.end() && atof(it->second.c_str()) < atof(myvec[i][j].c_str())) {

									if (j == 4) {
										signal4[it->first] = make_pair(myvec[i][1], mu);
									}
									else {
										int dt = atof((it->second).c_str()) - atof(myvec[i][j - 1].c_str());
										double signalsurvey = signal * pow(2, -dt / life);
										if (signalsurvey<theta) signalsurvey = mu;
										signal4[it->first] = make_pair(myvec[i][1], signalsurvey);
									}

								}
							}
							if (signal4.size() == surveytime.size()) {
								break;
							}
							if (signal == 0) signal = mu;
							else {
								int dt = atof(myvec[i][j].c_str()) - atof(myvec[i][j - 1].c_str());
								signal = signal * pow(2, -dt / life);
								if (signal <theta) {
									signal = mu;
								}
								else {
									signal = mu + signal * (1 - mu);
								}
							}
							if (j == nums + 2) {
								for (map<string, string>::const_iterator it = surveytime.begin();
									it != surveytime.end(); ++it)
								{
									if (signal4.find(it->first) == signal4.end()) {
										int dt = atof((it->second).c_str()) - atof(myvec[i][j].c_str());
										double signalsurvey = signal * pow(2, -dt / life);
										signal4[it->first] = make_pair(myvec[i][1], signalsurvey);

									}
								}
							}




						}

						for (map<string, pair<string, double> >::const_iterator it = signal4.begin();
							it != signal4.end(); ++it) {
							modifyvector(top4signals[it->first], signal4[it->first]);
						}
					}
					*/

					/* myfile1<<myvec[0][0]<<endl;
					myfile1<<mu<<' '<<life<<' '<<theta<<endl;
					for(map <string, vector<pair<string, double> > >::const_iterator it = top4signals.begin();
					it != top4signals.end(); ++it){
					myfile1<<it->first<<endl;
					for(int k = 0; k < it->second.size(); k++){
					myfile1<<it->second[k].first<<" "<<it->second[k].second<<" ";
					}myfile1<<endl;
					}*/

					//TODO, WE NEED KNOW HOW COMPARE WITH SURVEY-IN FILE
					/*
					for (map <string, vector<pair<string, double> > >::const_iterator it = top4signals.begin();
						it != top4signals.end(); ++it) {
						int index = findindex(mu, life, theta);
						double size1 = it->second.size();
						double size2 = partnersbysurvey[it->first].size();
						set<string> temp;
						for (int k = 0; k<it->second.size(); k++) {
							temp.insert(it->second[k].first);
						}
						for (int k = 0; k<partnersbysurvey[it->first].size(); k++) {
							temp.insert(to_string(partnersbysurvey[it->first][k]));
						}
						double Jaccard_score = (size1 + size2 - temp.size()) / temp.size();

						int times = Jaccardmap[it->first][index].first;
						double orivalue = Jaccardmap[it->first][index].second;
						Jaccardmap[it->first][index] = make_pair(times + 1, (orivalue*times + Jaccard_score) / (times + 1));
					}
					*/
				}
			}
		}
		if (end) break;
	}


	for (map<string, vector<pair<int, double> > >::const_iterator it = Jaccardmap.begin();
		it != Jaccardmap.end(); ++it) {
		myfile1 << "survey number    " << it->first << endl;
		for (int i = 0; i< it->second.size(); i++) {
			myfile1 << it->second[i].first << ' ' << it->second[i].second << endl;
		}
	}
	myfile1.close();
	cout << a << ' ' << b << ' ' << c << ' ' << d;


}
