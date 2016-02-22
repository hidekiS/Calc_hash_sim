/*
 * Mydebug.h
 *
 *  Created on: 2015/11/17
 *      Author: shiroshita
 */

#ifndef MYDEBUG_H_
#define MYDEBUG_H_

//デバッグ用のクラス　debug()を呼ぶたびに"debug x"が出力される
class Mydebug{
private:
	int i;
public:
	Mydebug(){
		i=1;
	}
	void set_i(int I){
		i=I;
	}
	void debug(){
		std::cout << "debug " << i++ << std::endl;
	}
	void debug(int I){
		std::cout << "debug " << I << std::endl;
		i = I+1;
	}
	void debug(char *str){
		std::cout << "debug " << str << std::endl;
	}
};

#endif /* MYDEBUG_H_ */
