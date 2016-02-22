/*
 * Calc_hash_sim.h
 *
 *  Created on: 2015/11/02
 *      Author: shiroshita
 */

#ifndef CALC_HASH_SIM_H_
#define CALC_HASH_SIM_H_

#include <iostream>
#include <stdio.h>
#include <string>
#include <typeinfo>
#include <fstream>
#include <vector>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include "Mydebug.h"
#include <bitset>
#define foreach BOOST_FOREACH
#define MINDIFFRATE_LEN 0.3 //ショット長の類似度がこれよりも小さい場合、類似度=0
#define MINDIFFRATE_LUM 0.4 //ショットの輝度値比による類似度がこれよりも小さい場合、類似度=0
#define MINDIFFRATE_HASH 0.3 //ハッシュ値の違いによる類似度の足切り
using namespace std;

//ショット情報の構造体
struct shot{
	size_t hash;
	size_t len_shot;
	double avg_lum;
};

// 比較演算子のオーバーロード
bool operator<(const shot& left, const shot& right) {
        return left.len_shot <  right.len_shot;
}
//動画データクラス
class ShotData{
private:
	string fname;
	size_t num_div;
	size_t min_frame;
	size_t num_shot;
	vector <shot> lshot;

public:
	ShotData(){
		fname = "";
		num_div=0;
		min_frame = 0;
		num_shot=0;
	}
	void setHeader(string Fname,size_t Num_div,size_t minFrame,size_t Num_shot){
		fname = Fname;
		num_div = Num_div;
		min_frame = minFrame;
		num_shot = Num_shot;
	}
	const string& getFname() const {
		return fname;
	}
	void setFname(const string fname) {
		this->fname = fname;
	}
	size_t getNumShot() const {
		return num_shot;
	}
	void setNumShot(size_t numShot) {
		num_shot = numShot;
	}
	size_t getNumDiv() const {
		return num_div;
	}
	void setNumDiv(size_t numDiv) {
		num_div = numDiv;
	}
	size_t getMinFreme() const {
		return min_frame;
	}
	void setMinFreme(size_t minFrame) {
		min_frame = minFrame;
	}
	vector<shot>& getShot(){
		return lshot;
	}
	void preprocess();
	void dump();
};

double calc_sim_time_series(vector<shot> Db,vector<shot> Key,size_t divnum);
double calc_sim_shot_len(vector<shot> Db,vector<shot> Key,size_t i,size_t j);
double calc_sim_hash(size_t div_num,vector<shot> Db,vector<shot> Key,size_t i,size_t j);
double calc_sim_lumA(vector<shot> Db,vector<shot> Key,size_t i,size_t j);
double calc_sim_lumB(vector<shot> Db,vector<shot> Key,size_t i,size_t j);
double calc_sim_shot_num(ShotData Db,ShotData Key);
void fwrite_sim(list< ShotData > lshotdata,
		list<double> lsim_hash,
		list<double> lsim_lum,
		list<double> lsim_shot_num,
		list<double> lsim_time_series,
		char *fname);
int get_lum_block(size_t hash);
#endif /* CALC_HASH_SIM_H_ */

