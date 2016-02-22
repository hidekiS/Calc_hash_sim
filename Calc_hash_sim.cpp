//============================================================================
// Name        : Calc_hash_sim.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "Calc_hash_sim.h"
#include "Mydebug.h"

int main(int argc, char **argv) {

	Mydebug md;
	//ofstreamクラスを使って動画情報ファイルを読み込みオープン
	ifstream ifs(argv[1],std::ios::in);
	if( !ifs ){
		cout << "Error: cannot open file(" << argv[1] << ")" << endl;
		return -1;
	}
	//read file
	string str;
	ShotData shotdata;
	list< ShotData > lshotdata;
	vector< string > lstr;
	char buf[1024];
	size_t header_size = 4;

	try{
		while(ifs.getline(buf,1024)){
			//sprit string
			string str(buf);
			boost::split(lstr, str, boost::is_any_of("\t"));
			shotdata.setHeader(lstr[0],boost::lexical_cast<size_t>(lstr[1]),boost::lexical_cast<size_t>(lstr[2]),boost::lexical_cast<size_t>(lstr[3]));
			shot st1;
			for(size_t i=0;i<shotdata.getNumShot();i++){
				st1.hash = boost::lexical_cast<size_t>(lstr[header_size+i]);
				st1.len_shot = boost::lexical_cast<size_t>(lstr[shotdata.getNumShot()+header_size+i]);
				st1.avg_lum = (boost::lexical_cast<double>(lstr[(shotdata.getNumShot()*2)+header_size+i]));
				shotdata.getShot().push_back(st1);
			}
			lshotdata.push_back(shotdata);
			shotdata.getShot().clear();
		}
	}catch(exception e){
		cout << "catch error" <<endl;
	}

	ifs.close();

	//calc time series sim
	list<double> lsim_time_series;

	BOOST_FOREACH(ShotData db, lshotdata){
		BOOST_FOREACH(ShotData key, lshotdata){
			lsim_time_series.push_back(calc_sim_time_series(db.getShot(),key.getShot(),key.getNumDiv()));
		}
	}

	//preprocess
	cout << "now preprocessing...";
	BOOST_FOREACH(ShotData &sdata, lshotdata){
		sdata.preprocess();	//バグありそう
	}
	cout << "finish" <<endl;

	//debug
	BOOST_FOREACH(ShotData sdata, lshotdata){
		sdata.dump();
	}

	//calc similarity
	cout << "calc similarity start...";
	//double sim_shot_len=0;
	double sim_hash=0;
	double sim_lum=0;

	list<double> lsim_shot_len;
	list<double> lsim_hash;
	list<double> lsim_lum;
	list<double> lsim_shot_num;

	BOOST_FOREACH(ShotData db, lshotdata){
		BOOST_FOREACH(ShotData key, lshotdata){
			double shot_sum = (db.getShot().size()*key.getShot().size());
			//double tmpW=0;
			for(size_t i=0;i<db.getShot().size();i++){
				for(size_t j=0;j<key.getShot().size();j++){
					double X;
					X=calc_sim_shot_len(db.getShot(),key.getShot(),i,j);
					//if(db.getNumDiv() == key.getNumDiv()){
					sim_hash += calc_sim_hash(db.getNumDiv(),db.getShot(),key.getShot(),i,j)*X;
					sim_lum += calc_sim_lumA(db.getShot(),key.getShot(),i,j)*X;
					//}else{ break;}
					//tmpW+=X;
				}
			}
			lsim_shot_len.push_back(0);
			lsim_hash.push_back(sim_hash/shot_sum);
			lsim_lum.push_back(sim_lum/shot_sum);

			//	sim_shot_len = 0;
			sim_hash=0;
			sim_lum=0;
			lsim_shot_num.push_back(calc_sim_shot_num(db,key));
		}
	}

	cout << "calc finish"<<endl;

	fwrite_sim(lshotdata,lsim_hash,lsim_lum,lsim_shot_num,lsim_time_series,argv[1]);

	return 0;
}
//時系列類似度を返す
double calc_sim_time_series(vector<shot> Db,vector<shot> Key,size_t divnum){
	//todo
	double simhash = 0;
	size_t db_len=0;
	size_t key_len=0;
	BOOST_FOREACH(shot dbst,Db){
		db_len += dbst.len_shot;
	}
	BOOST_FOREACH(shot kyst,Key){
		key_len += kyst.len_shot;
	}

	vector <double> db_shot_norm_len(Db.size());
	vector <double> key_shot_norm_len(Key.size());
	size_t time_unit = 120; //0-1間をいくつに分けるか
	vector <size_t> dbhash;
	vector <size_t> keyhash;

	for(size_t i=0;i<Db.size();i++){
		db_shot_norm_len[i] = (double)Db[i].len_shot/db_len;
		for(size_t j=0;j<time_unit*db_shot_norm_len[i];j++){
			dbhash.push_back(Db[i].hash);
		}
	}
	for(size_t i=0;i<Key.size();i++){
		key_shot_norm_len[i] = (double)Key[i].len_shot/key_len;
		for(size_t j=0;j<time_unit*key_shot_norm_len[i];j++){
			keyhash.push_back(Key[i].hash);
		}
	}

	//末尾をtime unitのサイズに合うように調整　クッソ汚いコード
	while(dbhash.size() > time_unit){
		dbhash.pop_back();
	}
	while(keyhash.size() > time_unit){
		keyhash.pop_back();
	}

	for(size_t i=0;i<time_unit;i++){
		simhash += (double)min(get_lum_block(dbhash[i]),get_lum_block(keyhash[i]));
	}
	//cout << simhash/time_unit <<endl;
	return simhash/time_unit;
}
int get_lum_block(size_t hash){
	bitset<64> bs(hash);
	int count=0;
	for(size_t i=0;i<bs.size();i++){
		if(bs[i] == 1){
			count++;
		}
	}
	return count;
}
//ショット長による類似度を返す
double calc_sim_shot_len(vector<shot> Db,vector<shot> Key,size_t i,size_t j){
	double sim_shot_len=0;
	//cout << Db[i].len_shot <<":"<< Key[j].len_shot << endl;
	if(Db[i].len_shot < Key[j].len_shot){
		sim_shot_len = (double)Db[i].len_shot / (double)Key[j].len_shot;
		if(sim_shot_len < MINDIFFRATE_LEN){
			return 0;
		}else{
			return sim_shot_len * (((double)i+1) + ((double)j+1));
		}
	}else{
		sim_shot_len = (double)Key[j].len_shot / (double)Db[i].len_shot;
		if(sim_shot_len < MINDIFFRATE_LEN){
			return 0;
		}else{
			return sim_shot_len * (((double)i+1)+ ((double)j+1));
		}
	}
}
//ハッシュ値による類似度を返す
double calc_sim_hash(size_t div_num,vector<shot> Db,vector<shot> Key,size_t i,size_t j){
	bitset<64> db((long)Db[i].hash);
	bitset<64> key((long)Key[j].hash);
	size_t count=0;
	for(size_t i=0;i<div_num;i++){
		if(db[i] != key[i]){
			count+=1;
		}
	}
	double sim;
	if((sim = 1-((double)count/(double)div_num)) <= 0.2){
		return 0;
	}
	else
		return sim;
}
//プランA 輝度値比による類似度
double calc_sim_lumA(vector<shot> Db,vector<shot> Key,size_t i,size_t j){
	double sim=0;
	//cout << Db[i].avg_lum <<":"<< Key[j].avg_lum << endl;
	if(Db[i].avg_lum < Key[j].avg_lum){
		sim = (double)Db[i].avg_lum / (double)Key[j].avg_lum;
		if(sim < MINDIFFRATE_LUM){
			return 0;
		}else{
			return sim;
		}
	}else{
		sim = (double)Key[j].avg_lum / (double)Db[i].avg_lum;
		if(sim < MINDIFFRATE_LUM){
			return 0;
		}else{
			return sim;
		}
	}
}
//プランB 輝度値差による類似度
double calc_sim_lumB(vector<shot> Db,vector<shot> Key,size_t i,size_t j){
	//cout << Db[i].avg_lum <<":"<< Key[j].avg_lum << endl;
	return 1-(fabs(Db[i].avg_lum-Key[j].avg_lum)/(Db[i].avg_lum+Key[j].avg_lum));
}
//ショット数による類似度
double calc_sim_shot_num(ShotData Db,ShotData Key){
	//cout << Db.getNumShot() <<":"<< Key.getNumShot() << endl;
	if(Db.getNumShot()<Key.getNumShot()){
		return (double)Db.getNumShot()/(double)Key.getNumShot();
	}else{
		return (double)Key.getNumShot()/(double)Db.getNumShot();
	}
}
void fwrite_sim(list< ShotData > lshotdata,
		list<double> lsim_hash,
		list<double> lsim_lum,
		list<double> lsim_shot_num,
		list<double> lsim_time_series,
		char *fname){

	string tmpstr(fname);
	list <string> ltmpstr;
	boost::split((ltmpstr), tmpstr, boost::is_any_of("/"));
	//ofstreamクラスを使って表計算ファイルを書き込みオープン
	vector <string> lstr;
	lstr.push_back("sim/sim_shot_hash_");
	lstr.push_back("sim/sim_shot_lum_");
	lstr.push_back("sim/sim_shot_num_");
	lstr.push_back("sim/sim_time_series");

	ofstream ofs[lstr.size()];
	for(size_t i=0;i<lstr.size();i++){
		lstr[i].append(ltmpstr.back());
		lstr[i].append(".ods");
		ofs[i].open(lstr[i].c_str(),ios::out);
		if( !ofs[i] ){
			cout << "Error: cannot open file(" << lstr[i] << ")" << endl;
			return ;
		}
	}

	//	list<double>::iterator shot_len_ite = lsim_shot_len.begin();
	//	list<double>::iterator hash_ite = lsim_hash.begin();
	//	list<double>::iterator lum_ite = lsim_lum.begin();
	//	list<double>::iterator shot_num_ite = lsim_shot_num.begin();

	list<double>::iterator ite[lstr.size()] = {lsim_hash.begin(),lsim_lum.begin(),lsim_shot_num.begin(),lsim_time_series.begin()};
	//キー動画のファイル名出力
	for(size_t i=0;i<lstr.size();i++){
		ofs[i] << " ";
		BOOST_FOREACH(ShotData key, lshotdata){
			ofs[i] << key.getFname() << "\t";
		}
		ofs[i] << endl;
	}

	//類似度の出力
	BOOST_FOREACH(ShotData db, lshotdata){
		for(size_t i=0;i<lstr.size();i++){
			ofs[i] << db.getFname() << "\t";
		}
		BOOST_FOREACH(ShotData key, lshotdata){
			for(size_t i=0;i<lstr.size();i++){
				ofs[i]<<*ite[i]<<"\t";
				ite[i]++;
			}
		}
		for(size_t i=0;i<lstr.size();i++){
			ofs[i] << endl;
		}
	}
	for(size_t i=0;i<lstr.size();i++){
		ofs[i].close();
	}
}

//不要なショットを削除する & ショット長順にソート
void ShotData::preprocess(){
	lshot.erase(lshot.begin()); //先頭を削除
	lshot.pop_back();	//末尾を削除

	for(vector< shot >::iterator ite = lshot.begin();ite != lshot.end();){
		if(ite->hash == 0 || ite->len_shot == min_frame){
			ite = lshot.erase(ite);	//ハッシュ値が0 or 最小ショット長のショットを削除
		}else{
			ite++;
		}
	}
	sort(lshot.begin(),lshot.end());
}
//インスタンスのデータを標準出力に
void ShotData::dump(){
	cout << "file name:"<<fname<<endl;
	cout << "div num:"<<num_div<<endl;
	cout << "MIN frame:"<<min_frame<<endl;
	cout << "shot num:"<<num_shot<<endl;
	for(size_t i=0;i<lshot.size();i++){
		cout << lshot[i].hash << "\t" << lshot[i].len_shot << "\t" << lshot[i].avg_lum <<endl;
	}
	cout <<endl;
}
