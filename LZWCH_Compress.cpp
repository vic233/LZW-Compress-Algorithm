#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <fstream>
#include <sys/stat.h>

using namespace std;

map <string, unsigned int> dict;  //编码词典，为了查找方便，将码字和缀-符串的位置对调
map <unsigned int, string> reDict;  //解码词典
map <unsigned int, int> dictFirstCharLength;  //词典中码字对应的缀-符串中第一个字符的二进制位数
vector<int> charBinLen;  //存储读入字符转为二进制后每个字节的位数，包括分隔符
vector<unsigned int> codeStream;  //码字流，用来存储编码生成的码字
vector<string> charStream;  //字符流，存储带解码待解码数据
vector<string> charStream_deCompress;  //解码字符流，存储解码时生成的字符
vector<int> codeToBinDigits;  //用来记录码字每次存储的二进制位数是多少
vector<bool> codeIsTruncated;  //码字是否被截断
bool isDoubleBytes = false;  //用来判断是否为双字节
int toBinWhetherDelimiter = 0;  //用来判断双字节中的低字节以添加分隔符
unsigned int reservedEmptyNum = 600;  //预留的空词典条目

string intToBin(int num);

int LZW_initDict()
{
	cout << "正在初始化词典..." << endl;
	int i;
	string s;
	int num = 20;
	int num_2;
	//将ASCII值的二进制全部加入基本码集
	for (i = 0; i < 128; i++)
	{
		s = intToBin(i);
		dict[s] = i;
		reDict[i] = s;
		s.clear();
	}

	//将GB2312编码的1区字符的二进制加入基本码集
	int gbNum = 41378;
	for (; i < 221; i++)
	{
		s = intToBin(gbNum);
		dict[s] = i;
		reDict[i] = s;
		s.clear();
		gbNum++;
	}

	//将GB2312编码的3区字符的二进制加入基本码集
	int gbNum_2 = 41889;
	for (; i < 315; i++)
	{
		s = intToBin(gbNum_2);
		dict[s] = i;
		reDict[i] = s;
		s.clear();
		gbNum_2++;
	}

	//将GB2312编码的一级汉字（16 - 55区）的二进制加入基本码集
	int gbNum_3 = 45217;
	int j = 1;
	for (; i < 4070; i++)
	{
		s = intToBin(gbNum_3);
		dict[s] = i;
		reDict[i] = s;
		s.clear();
		gbNum_3++;

		if (j == 94)
		{
			gbNum_3 += 162;
			j = 0;
		}
		j++;
	}

	//将GB2312编码的部分二级汉字（66 - 73区）的二进制加入基本码集
	int gbNum_4 = 58017;
	j = 1;
	for (; i < 4822; i++)
	{
		s = intToBin(gbNum_4);
		dict[s] = i;
		reDict[i] = s;
		s.clear();
		gbNum_4++;

		if (j == 94)
		{
			gbNum_4 += 162;
			j = 0;
		}
		j++;
	}

	//预留600个空项目加入基本码集
	num_2 = i + reservedEmptyNum;
	for (; i < num_2; i++)
	{
		num++;
		s = to_string(num);
		dict[s] = i;  //将非二进制形式的字符串加入基本码集以代表空项目
		reDict[i] = "";
		s.clear();
	}

	cout << "词典初始化完成！\n" << endl;

	return i;
}

void LZW_compress(unsigned int init_num)
{
	cout << "正在编码..." << endl;
	unsigned int code;  //词典中的码字
	unsigned int codeEmpty;  //词典中预留项的码字
	int len;  //词典缀-符串中第一个字符的二进制位数
	int i = 0;
	int j = 0;
	//unsigned int initNumExceptEmpty = init_num - reservedEmptyNum;
	int num = 20;
	string s;
	string currentPrefix;  //初始时当前前缀P为空
	string currentChar;  //当前字符C
	string str;  //缀-符串S
	map<string, unsigned int>::iterator iter_d;
	map<string, unsigned int>::iterator iter_d_2;
	map<string, unsigned int>::iterator iter_d_3;
	vector<string>::iterator iter_c;

	for (iter_c = charStream.begin(); iter_c != charStream.end(); iter_c++)
	{
		currentChar = *iter_c;  //当前字符C:=字符流中的下一个字符
		if (i == 0)
		{
			len = currentChar.length();  //获取词典缀-符串中第一个字符的二进制位数
			i++;
		}
		iter_d = dict.find(currentChar);  //找到代表C的码字
		if (iter_d == dict.end())  //C不在基本码集中，将其加入预留项中
		{
			num++;
			s = to_string(num);
			//cout << s << " ";
			iter_d_2 = dict.find(s);  //找到要插入的预留项的位置
			if (iter_d_2 != dict.end())
			{
				code = iter_d_2->second;  //存储该预留项的码字
				 //cout << "\ncurrentChar = " << currentChar << endl;
				//cout << "codeEmpty = " << code << " " << initNumExceptEmpty + j << endl;
				dict.erase(iter_d_2);  //删除该项
				dict.insert(pair<string, unsigned int>(currentChar, code));  //将待添加缀-符串加入编码词典的预留项中
				//cout << "dict[" << currentChar << "] = " << dict[currentChar] << endl;

				str = currentPrefix + currentChar;  //缀-符串S=P+C
				//cout << "currentPrefix = " << currentPrefix << " currentChar = " << currentChar << " str = " << str << endl;
				iter_d_3 = dict.find(str);

				reDict[code] = currentChar;  //将待添加缀-符串加入解码词典的预留项中
			}
			else
			{
				cout << "预留的空词典条目未正常添加，压缩出错" << endl;
				exit(EXIT_FAILURE);
			}
			j++;
		}
		else
		{
			str = currentPrefix + currentChar;  //缀-符串S=P+C
			//cout << "\ncurrentPrefix = " << currentPrefix << " currentChar = " << currentChar << " str = " << str << endl;
			iter_d_3 = dict.find(str);
		}

		/*if (init_num == 6241)
		{
			cout << "currentChar: " << currentChar << " currentPrefix: " << currentPrefix << endl;
			cout << "str: " << str << endl;
		}*/

		//判断S是否在词典中
		if (iter_d_3 != dict.end())  //找到了
		{
			currentPrefix = str;  //P:=P+C
			//cout << "str to currentPrefix = " << currentPrefix << endl;
		}
		else  //S不在词典中
		{
			iter_d_3 = dict.find(currentPrefix);  //找到代表P的码字
			code = iter_d_3->second;
			codeStream.push_back(code);  //把码字输出到码字流中
			dictFirstCharLength.insert(pair<unsigned int, int>(code, len));
			//cout << "code = " << code << " len = " << len << endl;
			/*if (code == 4079)
			{
				cout << "currentPrefix = " << currentPrefix << " currentChar = " << currentChar << endl;
				cout << "init_num = " << init_num << "  str = " << str << endl;
				cout << "code = " << code << " len = " << len << endl;
			}*/
			dict.insert(pair<string, unsigned int>(str, init_num));  //把缀-符串S添加到词典
			//cout << "init_num = " << init_num << "  str = " << str << endl;

			//if (init_num == 6721)
			//{
			//	//cout << "init_num = " << init_num << " str = " << str << endl;
			//	cout << "currentChar = " << currentChar << endl;
			//	cout << "code = " << code << " len = " << len << endl;
			//}

			init_num++;  //码值加1
			if (init_num > 4294967295)
			{
				cout << "词典条目超过限制" << endl;
				exit(EXIT_FAILURE);
			}
			currentPrefix = currentChar;  //令P:=C
			len = currentChar.length();  //获取词典缀-符串中第一个字符的二进制位数
		}
	}
	iter_d_3 = dict.find(currentPrefix);  //找到代表P的码字
	code = iter_d_3->second;
	codeStream.push_back(code);  //把码字输出到码字流中
	dictFirstCharLength.insert(pair<unsigned int, int>(code, len));
	//cout << "code = " << code << " len = " << len << endl;


	/*for (iter_d = dict.begin(); iter_d != dict.end(); iter_d++)
	{
		cout << iter_d->first << " " << iter_d->second << endl;
	}*/

	cout << "编码完成！" << endl;
	//cout << "当前vector分配的大小: " << codeStream.capacity() << "   " << "当前使用数据的大小: " << codeStream.size() << endl << endl;
}

void LZW_deCompress(unsigned int init_num)
{
	//map<unsigned int, string> reDict_2;
	cout << "正在解码..." << endl;
	int len;  //词典缀-符串中第一个字符的二进制长度
	unsigned int code = init_num;  //词典中的码字
	string currentPrefix;  //初始时当前前缀P为空
	string currentChar;  //当前字符C
	unsigned int currentCodeWord;  //当前码字cW
	unsigned int previousCodeWord;  //先前码字cW
	string str;  //缀-符串S
	string currentStr;  //当前缀-符串cS
	string currentStr_temp;  //没有分隔符的当前缀-符串cS
	string previousStr;  //先前缀-符串pS
	unsigned int codeStream_len = codeStream.size();
	map<unsigned int, int>::iterator iter_dL;

	currentCodeWord = codeStream[0];  //当前码字cW:=码字流中的第一个码字
	currentStr = reDict[currentCodeWord];  //保存cW指向的缀-符串S为当前缀-符串cS
	//cout << "currentStr = " << currentStr << endl;
	//iter_dL = dictFirstCharLength.find(currentCodeWord);  //找到当前码字对应的缀-符串中第一个字符的二进制位数
	//len = iter_dL->second;
	//if (len == 16)
	//{
	//	currentStr.erase(len / 2, 1);  //将分隔符删除以便输出
	//}
	//else
	//{
	//	currentStr.erase(len, 1);  //将分隔符删除以便输出
	//}
	//cout << "currentStr = " << currentStr << " len = " << len << endl;
	charStream_deCompress.push_back(currentStr);  //输出当前缀-符串cS到字符流

	vector<unsigned int>::iterator iter_c;
	/*
	unsigned int m = 0;
	for (iter_c = codeStream.begin(); iter_c != codeStream.end(); iter_c++)
	{
		cout << m << ":" << *iter_c << " ";
		m++;
	}
	cout << endl;
	*/

	unsigned int i = 1;
	for (iter_c = codeStream.begin() + 1; iter_c != codeStream.end(); iter_c++)
	{
		previousCodeWord = currentCodeWord;  //先前码字pW:=当前码字cW
		currentCodeWord = *iter_c;  //cW:=码字流中的下一个码字
		/*
		if (code == 273)
		{
			cout << "i: " << i << " currentCodeWord: " << currentCodeWord << " currentPrefix : " << currentPrefix << " currentChar : " << currentChar << endl;
			cout << "currentStr = " << currentStr << endl;
			cout << "reDict[271]: " << reDict[271] << endl;
			cout << "codeStream[146]: " << codeStream[146] << endl;
		}
		*/
		if (reDict[currentCodeWord] != "")  //当前缀-符串在词典中
		{
			//cout << "currentCodeWord = " << currentCodeWord << endl;
			currentStr = reDict[currentCodeWord];  //保存cW指向的缀-符串S为当前缀-符串cS
			//cout << "currentStr = " << currentStr << endl;
			iter_dL = dictFirstCharLength.find(currentCodeWord);  //找到当前码字对应的缀-符串中第一个字符的二进制位数
			len = iter_dL->second;
			//if (len == 16)
			//{
			//	currentStr.erase(len / 2, 1);  //将分隔符删除以便输出
			//}
			//else
			//{
			//	currentStr.erase(len, 1);  //将分隔符删除以便输出
			//}
			//cout << "currentStr = " << currentStr << " len = " << len << endl;
			charStream_deCompress.push_back(currentStr);  //输出当前缀-符串到字符流
			//cout << "previousCodeWord = " << previousCodeWord << endl;
			previousStr = reDict[previousCodeWord];  //保存pW指向的缀-符串S为先前缀-符串pS
			//cout << "previousStr = " << previousStr << endl;
			currentPrefix = previousStr;  //P:=pS
			//cout << "currentPrefix = " << currentPrefix << endl;
			currentChar = currentStr.substr(0, len);  //当前字符C:=当前缀-符串cS的第一个字符
			//cout << "currentChar = " << currentChar << endl;
			str = currentPrefix + currentChar;  //缀-符串S=P+C
			//cout << "code = " << code << "  str = " << str << endl;
			reDict.insert(pair<unsigned int, string>(code, str));  //把缀-符串S添加到词典
			//reDict[code] = str;  //把缀-符串S添加到词典

			/*if (code == 4102)
			{
				cout << "i = " << i << endl;
				cout << "previousCodeWord = " << previousCodeWord << " currentCodeWord = " << currentCodeWord << endl;
				cout << "currentStr = " << currentStr << endl;
				cout << "reDict[currentCodeWord] = " << reDict[currentCodeWord] << endl;
				cout << "currentPrefix = " << currentPrefix << " currentChar = " << currentChar << endl;
				cout << "str = " << str << endl;
				cout << "len = " << len << " dictFirstCharLength[" << currentCodeWord << "] = " << dictFirstCharLength[currentCodeWord] << endl;
			}*/

			//cout << "code = " << code << endl << endl;

			/*if (i == 52)
			{
				cout << "previousCodeWord: " << previousCodeWord << " currentCodeWord: " << currentCodeWord << endl;
				cout << "currentPrefix: " << currentPrefix << " currentStr: " << currentStr << endl;
				cout << "code: " << code << " str: " << str << endl;
				cout << reDict[52] << endl;
			}*/

			code++;  //码值加1
			i++;
		}
		else  //当前缀-符串不在词典中
		{
			//cout << "currentCodeWord = " << currentCodeWord << endl;
			//cout << "previousCodeWord = " << previousCodeWord << endl;
			previousStr = reDict[previousCodeWord];  //保存pW指向的缀-符串S为先前缀-符串pS
			//cout << "previousStr = " << previousStr << endl;
			currentPrefix = previousStr;  //P:=pS
			//cout << "currentPrefix = " << currentPrefix << endl;
			iter_dL = dictFirstCharLength.find(previousCodeWord);  //找到先前码字对应的缀-符串中第一个字符的二进制位数
			len = iter_dL->second;
			//if (len == 16)
			//{
			//	previousStr.erase(len/2, 1);  //将分隔符删除以便输出
			//}
			//else
			//{
			//	previousStr.erase(len, 1);  //将分隔符删除以便输出
			//}
			//cout << "currentPrefix = " << currentPrefix << " len = " << len << endl;
			currentChar = currentPrefix.substr(0, len);  //当前字符C:=当前缀-符串cS的第一个字符
			//cout << "currentChar = " << currentChar << endl;
			str = currentPrefix + currentChar;  //缀-符串S=P+C，未去除分隔符，用于词典
			charStream_deCompress.push_back(str);  //输出当缀-符串S到字符流
			//cout << " :code = " << code << "  str = " << str << endl;
			//reDict.insert(pair<unsigned int, string>(code, str));  //把缀-符串S添加到词典，此处有BUG，无法正常插入
			reDict[code] = str;  //把缀-符串S添加到词典

			/*if (code == 4102)
			{
				cout << "i = " << i << endl;
				cout << "previousCodeWord = " << previousCodeWord << " currentCodeWord = " << currentCodeWord << endl;
				cout << "currentStr = " << currentStr << endl;
				cout << "reDict[currentCodeWord] = " << reDict[currentCodeWord] << endl;
				cout << "currentPrefix = " << currentPrefix << " currentChar = " << currentChar << endl;
				cout << "str = " << str << endl;
				cout << "len = " << len << " dictFirstCharLength[" << currentCodeWord << "] = " << dictFirstCharLength[currentCodeWord] << endl;
			}*/

			//cout << ":code = " << code << endl << endl;
			/*
			if (i == 2539)
			{
				cout << "currentPrefix: " << currentPrefix << " currentChar: " << currentChar << endl;
				cout << ":code: " << code << " " << "str: " << str << endl;
			}
			*/
			code++;  //码值加1
			i++;
		}
	}

	/*map<unsigned int, string>::iterator iter_r;
	for (iter_r = reDict.begin(); iter_r != reDict.end(); iter_r++)
	{
		cout << iter_r->first << " " << iter_r->second << endl;
	}*/

	cout << "解码完成！" << endl;
}

string intToBin(int num)  //获得编码文本的二进制
{
	int temp;
	string bina;
	temp = num;
	if (num > 128)
	{
		isDoubleBytes = true;
	}
	if (num == 0)  //返回0的二进制0
	{
		bina = "0";
		bina += '|';
		return bina;
	}
	else if (num < 128 && isDoubleBytes != true)  //返回ASCII码的二进制
	{
		while (temp != 0)
		{
			bina = to_string(temp % 2) + bina;
			temp = temp >> 1;
		}
		bina += '|';  //为单字节后加上分隔符
		return bina;
	}
	else if (num <= 255 && isDoubleBytes == true)  //逐个返回双字节的高、低字节
	{
		while (temp != 0)
		{
			bina = to_string(temp % 2) + bina;
			temp = temp >> 1;
		}
		if (toBinWhetherDelimiter % 2 == 0)  //为高字节时不添加分割符
		{
			toBinWhetherDelimiter = 0;
			toBinWhetherDelimiter++;
		}
		else
		{
			bina = bina + '|';  //为低字节后加上分割符
			toBinWhetherDelimiter++;
			isDoubleBytes = false;
		}
		return bina;
	}
	else
	{
		int i = 1;
		while (temp != 0)
		{
			bina = to_string(temp % 2) + bina;
			temp = temp >> 1;
			if (i == 1)
			{
				bina = bina + '|';
			}
			i++;
		}
		isDoubleBytes = false;
		return bina;
	}
}

int numOfBytes(unsigned int num, string& binary)  //获得码字序列的二进制位数
{
	unsigned int temp;
	int codeToBinDigits;  //最大的码字序列转为二进制后的位数
	temp = num;
	if (temp == 0)
	{
		binary = "0";
	}
	while (temp != 0)
	{
		binary = to_string(temp % 2) + binary;
		temp = temp >> 1;
	}
	codeToBinDigits = binary.size();
	return codeToBinDigits;
}

unsigned int comResultWrite(string filename)
{
	char charStream_temp;
	unsigned char charStream_temp_2[2];
	string charStream_temp_3;
	int charWord_num;
	int len;  //单字节或双字节中的高字节的二进制长度
	int len_2;  //双字节中的低字节的二进制长度
	ifstream inFile;
	//读取存储字符流的二进制以便开始编码
	inFile.open(filename.c_str(), ios::binary);  //打开要编码的文件
	if (!inFile.is_open())
	{
		cout << "打开文件“" << filename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		//int i = 0;
		while (inFile.read(&charStream_temp, sizeof(charStream_temp)))  //输出字符流到变量
		{
			charStream_temp_2[0] = charStream_temp;
			if (charStream_temp_2[0] > 128)  //高字节大于0x81说明为GB2312或GBK编码，本程序基本码集虽然只包括部分GB2312，但对GBK也有效
			{
				inFile.read(&charStream_temp, sizeof(charStream_temp));
				charStream_temp_2[1] = charStream_temp;  //读入双字节的低字节
				//cout << int(charStream_temp_2[0]) << " " << int(charStream_temp_2[1]) << endl;
				charWord_num = charStream_temp_2[0];
				charStream_temp_3 = intToBin(charWord_num);  //将高字节转为二进制
				len = charStream_temp_3.length();  //获取高字节的二进制位数
				//cout << "high len = " << len << " high byte = " << charStream_temp_3 << endl;
				charBinLen.push_back(len);  //存储获取高字节的二进制位数
				//cout << intToBin(charWord_num) << endl;
				charWord_num = charStream_temp_2[1];
				charStream_temp_3.append(intToBin(charWord_num));  //将低字节转为二进制后与高字节拼接
				charStream.push_back(charStream_temp_3);  //拼接后的结果存入字符流
				len_2 = charStream_temp_3.length() - len;  //获取低字节的二进制位数，包括分隔符
				//cout << "low len = " << len_2 << " double byte = " << charStream_temp_3 << endl;
				charBinLen.push_back(len_2);  //存储获取低字节的二进制位数
				//cout << atoi(charStream_temp_3.c_str()) << endl;
				//cout << hex << int(charStream_temp) << " " << int(charStream_temp_2) << endl;
			}
			else  //此时为ASCII编码
			{
				charWord_num = charStream_temp_2[0];
				charStream_temp_3 = intToBin(charWord_num);  //将单字节转为二进制
				charStream.push_back(charStream_temp_3);  //将二进制结果存入字符流
				len = charStream_temp_3.length();  //获取单字节的二进制位数，包括分隔符
				//cout << "len = " << len << " byte = " << charStream_temp_3 << endl;
				charBinLen.push_back(len);  //存储获取的二进制位数
			}
			//charStream.push_back(charStream_temp);
			//cout << charStream[i] << endl;
			//i++;
		}
		cout << "\n读取待编码文件成功...\n" << endl;
	}
	inFile.close();
	inFile.clear();

	dict.clear();
	reDict.clear();

	unsigned int init_num = LZW_initDict();//词典初始化后的大小
	LZW_compress(init_num);

	/*
	unsigned int m = 0;
	vector<unsigned int>::iterator iter;
	cout << "编码后的结果为：" << endl;
	for (iter = codeStream.begin(); iter != codeStream.end(); iter++)
	{
		cout << m << ":" << * iter << " ";
		m++;
	}
	cout << endl << endl;
	*/

	ofstream outFile;
	string compressResultFilename = filename + ".lzwc";  //存储编码结果的文件名加后缀.lzwc
	//将编码结果写入到文件中
	outFile.open(compressResultFilename.c_str(), ios::binary);  //以二进制模式打开文件
	if (!outFile.is_open())
	{
		cout << "打开文件“" << compressResultFilename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		vector<unsigned int>::iterator iter_c;
		unsigned int i = 0;
		int len = 32;  //一个int变量的存储空间中未使用的位数
		int len_2;  //超过上个存储空间的位数
		int binDigits;  //码字所占二进制位数
		string binary;  //码字的二进制
		string binTemp = "";  //紧凑存储时拼接的二进制
		unsigned C_binary = 0;
		for (iter_c = codeStream.begin(); iter_c != codeStream.end(); iter_c++)
		{
			C_binary = *iter_c;  //将码字流中的码字逐个处理
			//cout << hex << C_binary << " ";
			binary = "";
			binDigits = numOfBytes(C_binary, binary);
			//cout << "code: " << *iter_c << endl;
			if (len - binDigits > 0)  //码字的二进制位数小于一个int变量的存储空间中未使用的位数
			{
				len -= binDigits;  //还剩余的位数
				binTemp += binary;  //紧凑存储时拼接的二进制
				//cout << " 1 " << endl;
				//cout << "binDigits = " << binDigits << " len = " << len << endl;
				//cout << "binary = " << binary << " binTemp = " << binTemp << endl << endl;
				codeToBinDigits.push_back(binDigits);  //将该码字的二进制位数存入数组
				codeIsTruncated.push_back(false);
				i++;
			}
			else if (len - binDigits == 0)  //码字的二进制位数等于于一个int变量的存储空间中未使用的位数
			{
				len -= binDigits;  //还剩余的位数，此时为0
				binTemp += binary;  //紧凑存储时拼接的二进制
				//cout << " 2 " << endl;
				//cout << "binDigits = " << binDigits << " len = " << len << endl;
				//cout << "binary = " << binary << " binTemp = " << binTemp << endl;
				try
				{
					C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
				}
				catch (std::out_of_range&)
				{
					cout << "1 out_of_range" << endl;
					exit(EXIT_FAILURE);
				}
				//cout << "C_binary = " << C_binary << endl << endl;
				outFile.write(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //以二进制形式将码字逐个写入到文件中
				codeToBinDigits.push_back(binDigits);  //将该码字的二进制位数存入数组
				codeIsTruncated.push_back(false);
				i++;
				len = 32;  //将未使用的位数还原
				binTemp.clear();  //清空临时变量
			}
			else  //码字的二进制位数大于一个int变量的存储空间中未使用的位数
			{
				binTemp += binary.substr(0, len);  //从高位开始，将未超过的二进制部分拼接
				//cout << " 3 " << endl;
				//cout << "binDigits = " << binDigits << " len = " << len << endl;
				//cout << "binary = " << binary << " binTemp = " << binTemp << endl;
				try
				{
					C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
				}
				catch (std::out_of_range&)
				{
					cout << "2 out_of_range" << endl;
					//cout << "binDigits = " << binDigits << " len = " << len << endl;
					//cout << "binary = " << binary << " binTemp = " << binTemp << endl;
					exit(EXIT_FAILURE);
				}
				//cout << "C_binary = " << C_binary << endl;
				outFile.write(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //以二进制形式将码字逐个写入到文件中
				codeToBinDigits.push_back(len);  //将该码字的未超出的二进制位数存入数组
				codeIsTruncated.push_back(true);
				i++;
				binary.erase(0, len);  //从高位开始，将未超过的二进制部分删除
				len_2 = binDigits - len;  //超过上个存储空间的位数
				binTemp = binary;  //剩余的二进制位数不可能超过32，所以直接存入临时变量
				//cout << "after erase,binary = " << binary << " binTemp = " << binTemp << endl;
				codeToBinDigits.push_back(len_2);  //将该码字还未处理的二进制位数存入数组
				codeIsTruncated.push_back(false);
				i++;
				len = 32 - len_2;  //还剩余的位数
				//cout << "len_2 = " << len_2 << " len = " << len << endl << endl;
			}
		}
		if (len != 32)  //len!=32说明不是在刚凑满一个存储空间时结束，此时还有数据没有写入文件
		{
			binTemp.append(len, '0');  //在临时变量后加0，使其位数增至32位
			//cout << "binTemp append 0 = " << binTemp << endl;
			try
			{
				C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
			}
			catch (std::out_of_range&)
			{
				cout << "3 out_of_range" << endl;
				exit(EXIT_FAILURE);
			}
			//cout << "C_binary = " << C_binary << endl;
			outFile.write(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //以二进制形式将码字逐个写入到文件中
			//codeToBinDigits[i - 1] += len;  //最后一个数据存储时占满了整个存储空间
			//cout << "last data's digits = " << codeToBinDigits[i - 1] << endl << endl;
		}
		//cout << "codeToBinDigits's length = " << codeToBinDigits.size() << " " << i -1 << endl << endl;
		cout << "编码结果写入“" << compressResultFilename << "”成功" << endl;
	}
	outFile.close();
	outFile.clear();
	return init_num;
}

void deComResultWrite(string filename, string compressResultFilename, unsigned int init_num)
{
	ifstream inFile;
	codeStream.clear();
	//读取编码结果文件以便解码
	inFile.open(compressResultFilename.c_str(), ios::binary);
	if (!inFile.is_open())
	{
		cout << "打开文件“" << compressResultFilename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		unsigned int i = 0;
		int len;  //码字每次存储的二进制位数
		int len_2;  //在二进制首部应添加的0的位数
		string binary = "";  //读取出的数据的二进制
		string binTemp = "";  //紧凑存储时拼接的二进制
		int binDigits;  //数据所占二进制位数
		unsigned int C_binary = 0;
		vector<int>::iterator iter;
		inFile.read(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //以二进制读取待解码码字数据
		binDigits = numOfBytes(C_binary, binary);  //将读取到的数据转为二进制
		//cout << "读取的数据 = " << C_binary << " 转为二进制后 = " << binary << endl;
		if (binDigits < 32)  //二进制未满32位，说明前面有0被省略
		{
			len_2 = 32 - binDigits;
			binary.insert(0, len_2, '0');  //加0
		}
		for (iter = codeToBinDigits.begin(); iter != codeToBinDigits.end(); iter++)
		{
			len = *iter;
			//cout << "len = " << len << endl;
			if (codeIsTruncated[i] == false && binary != "")  //能正常处理一个存储空间的数据
			{
				//cout << " 1 " << endl;
				binTemp = binary.substr(0, len);
				//cout << "binTemp = " << binTemp << " binary = " << binary << endl;
				binary.erase(0, len);  //删除已处理数据
				//cout << "after erase,binary = " << binary << endl;
				try
				{
					C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
				}
				catch (std::out_of_range&)
				{
					cout << "4 out_of_range" << endl;
					exit(EXIT_FAILURE);
				}
				codeStream.push_back(C_binary);
				//cout << "C_binary = " << C_binary << endl << endl;
				i++;
			}
			else if (codeIsTruncated[i] == false && binary == "")  //处理完了一个存储空间的数据且没有截断
			{
				//cout << " 2 " << endl;
				C_binary = 0;
				inFile.read(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //读取新的待解码码字数据
				binDigits = numOfBytes(C_binary, binary);  //将读取到的数据转为二进制
				if (binDigits < 32)  //二进制未满32位，说明前面有0被省略
				{
					len_2 = 32 - binDigits;
					binary.insert(0, len_2, '0');  //加0
				}
				//cout << "读取的数据 = " << C_binary << " 转为二进制后 = " << binary << endl;
				binTemp = binary.substr(0, len);
				//cout << "binTemp = " << binTemp << " binary = " << binary << endl;
				binary.erase(0, len);  //删除已处理数据
				//cout << "after erase,binary = " << binary << endl;
				try
				{
					C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
				}
				catch (std::out_of_range&)
				{
					cout << "5 out_of_range" << endl;
					exit(EXIT_FAILURE);
				}
				codeStream.push_back(C_binary);
				//cout << "C_binary = " << C_binary << endl << endl;
				i++;
			}
			else if (codeIsTruncated[i] == true && binary != "")  //处理到了截断的数据的上半部分
			{
				//cout << " 3 " << endl;
				binTemp = binary.substr(0, len);  //获取被截断数据的上半部分
				//cout << "binTemp = " << binTemp << " binary = " << binary << endl;
				binary.erase(0, len);  //删除已处理数据
				//cout << "after erase,binary = " << binary << endl;
				C_binary = 0;
				inFile.read(reinterpret_cast<char*>(&C_binary), sizeof(C_binary));  //读取新的待解码码字数据
				binDigits = numOfBytes(C_binary, binary);  //将读取到的数据转为二进制
				if (binDigits < 32)  //二进制未满32位，说明前面有0被省略
				{
					len_2 = 32 - binDigits;
					binary.insert(0, len_2, '0');  //加0
				}
				//cout << "读取的数据 = " << C_binary << " 转为二进制后 = " << binary << endl;
				i++;
				iter++;
				len = *iter;
				//cout << "len = " << len << endl;
				binTemp += binary.substr(0, len);  //获取被截断数据的下半部分
				//cout << "binTemp = " << binTemp << " binary = " << binary << endl;
				binary.erase(0, len);  //删除已处理数据
				//cout << "after erase,binary = " << binary << endl;
				try
				{
					C_binary = stoul(binTemp, 0, 2);  //将拼接的32位二进制转为十进制
				}
				catch (std::out_of_range&)
				{
					cout << "6 out_of_range" << endl;
					exit(EXIT_FAILURE);
				}
				codeStream.push_back(C_binary);
				//cout << "C_binary = " << C_binary << endl << endl;
				i++;
			}
			else
			{
				cout << "读取码字数据错误" << endl;
				exit(EXIT_FAILURE);
			}
			//cout << codeStream[i - 1] << " " << i << "  " << inFile.gcount() << " " << len << endl;
		}
		cout << "\n读取待解码文件成功...\n" << endl;
	}
	inFile.close();
	inFile.clear();

	LZW_deCompress(init_num);

	string C_word_temp;  //字符流中的每一项
	string C_word;  //字符流中每一项的每个单字节
	string C_lastBit;  //字符流中每一项中每个字节的最后一位
	int C_wordNum;  //字符流中每一项的每个字符的二进制（高字节和低字节）转十进制的结果
	int len_2;  //字符流中每一项的二进制位数
	int len_3;  //字符流中每一项的单字节二进制位数
	int len_4;
	int pos;
	int bin_ByteDigits;  //二进制所占的字节数
	string deCompressResultFilename = "lzwd_" + filename;  //存储解码结果的文件名加前缀lzwd_
	//写入解码结果到文件中
	ofstream outFile;
	outFile.open(deCompressResultFilename.c_str(), ios::binary);
	if (!outFile.is_open())
	{
		cout << "打开文件“" << deCompressResultFilename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		vector<int>::iterator iter_cL;
		unsigned int i;
		int j = 0;
		for (i = 0; i < charStream_deCompress.size(); i++)
		{
			//cout << "i: " << i << " ";
			C_word_temp = charStream_deCompress[i];
			//cout << "C_word_temp = " << C_word_temp << endl;
			len_2 = C_word_temp.length();  //获取字符流中每一项的长度
			//cout << "len_2: " << len_2 << endl;
			len_3 = charBinLen[j];  //获取读入时每个字节的二进制位数，包括分割符
			//cout << "len_3: " << len_3 << endl;
			while (len_2 - len_3 != 0)  //解码字符流中包含多个字节
			{
				if (len_3 == 0 && len_2 != 0)
				{
					cout << "\n写入文件的字符数超出范围，解压出错" << endl;
					exit(EXIT_FAILURE);
				}
				pos = len_3 - 1;
				C_lastBit = C_word_temp[pos];  //获取字符流中每一项中每个字节的最后一位
				//cout << "len_3 = " << len_3 << " C_lastBit = " << C_lastBit << endl;
				if (len_3 <= 8 && C_lastBit == "|")  //为单字节，包括分割符
				{
					len_4 = len_3 - 1;
					C_word = C_word_temp.substr(0, len_4);  //获取字符流中的单字节，不包括分割符
					//cout << "C_word = " << C_word << endl;
					try
					{
						C_wordNum = stoi(C_word, 0, 2);  //将二进制转为十进制
					}
					catch (std::invalid_argument&)
					{
						cout << "1 Invalid_argument" << endl;
						cout << "C_word = " << C_word << endl;
						exit(EXIT_FAILURE);
					}
					bin_ByteDigits = len_4 / 8 + 1;
					outFile.write(reinterpret_cast<char*>(&C_wordNum), bin_ByteDigits);
					C_word_temp.erase(0, len_3);  //将处理过的单字节删除，包括分割符
					len_2 -= len_3;  //减去处理过的字节长度，包括分割符
				}
				else if (len_3 == 8 && C_lastBit != "|")  //为高字节
				{
					C_word = C_word_temp.substr(0, len_3);  //获取字符流中的单字节
					//cout << "high C_word = " << C_word << endl;
					try
					{
						C_wordNum = stoi(C_word, 0, 2);  //将二进制转为十进制
					}
					catch (std::invalid_argument&)
					{
						cout << "2 Invalid_argument" << endl;
						cout << "C_word = " << C_word << endl;
						exit(EXIT_FAILURE);
					}
					bin_ByteDigits = len_3 / 8;
					outFile.write(reinterpret_cast<char*>(&C_wordNum), bin_ByteDigits);
					C_word_temp.erase(0, len_3);  //将处理过的单字节删除
					len_2 -= len_3;  //减去处理过的字节长度
				}
				else  //为低字节，包括分割符
				{
					len_4 = len_3 - 1;
					C_word = C_word_temp.substr(0, len_4);  //获取字符流中的单字节，不包括分割符
					//cout << "low C_word = " << C_word << endl;
					try
					{
						C_wordNum = stoi(C_word, 0, 2);  //将二进制转为十进制
					}
					catch (std::invalid_argument&)
					{
						cout << "3 Invalid_argument" << endl;
						cout << "C_word = " << C_word << endl;
						exit(EXIT_FAILURE);
					}
					bin_ByteDigits = len_4 / 8;
					outFile.write(reinterpret_cast<char*>(&C_wordNum), bin_ByteDigits);
					C_word_temp.erase(0, len_3);  //将处理过的单字节删除，包括分割符
					len_2 -= len_3;  //减去处理过的字节长度，包括分割符
				}
				j++;
				len_3 = charBinLen[j];  //获取下一个字节的长度
				//cout << "len_3: " << len_3 << endl;
			}
			if (len_2 - len_3 == 0)  //解码字符流中只有一个字节
			{
				len_4 = len_3 - 1;
				C_word = C_word_temp.substr(0, len_4);  //获取字符流中最后一个字节，不包括分割符
				//cout << "last C_word = " << C_word << endl;
				try
				{
					C_wordNum = stoi(C_word, 0, 2);  //将二进制转为十进制
				}
				catch (std::invalid_argument&)
				{
					cout << "4 Invalid_argument" << endl;
					cout << "C_word = " << C_word << endl;
					exit(EXIT_FAILURE);
				}
				if (len_4 % 8 != 0)
				{
					bin_ByteDigits = len_4 / 8 + 1;
				}
				else
				{
					bin_ByteDigits = len_4 / 8;
				}
				outFile.write(reinterpret_cast<char*>(&C_wordNum), bin_ByteDigits);
				j++;
			}
		}
		cout << "解码结果写入“" << deCompressResultFilename << "”成功" << endl;
	}
	outFile.close();
	outFile.clear();
}

size_t getFileSize(const char* fileName)
{

	if (fileName == NULL)
	{
		cout << "请输入文件名" << endl;
		exit(0);
	}
	struct stat statbuf;  // 这是一个存储文件(夹)信息的结构体，其中有文件大小和创建时间、访问时间、修改时间等
	stat(fileName, &statbuf);  // 提供文件名字符串，获得文件属性结构体
	double filesize = statbuf.st_size;// 获取文件大小

	return filesize;
}

int main()
{
	char option;
	string charStream_temp;  //要编码的字符流
	string filename;
	unsigned int init_num;
	dict.clear();
	reDict.clear();

	cout << "c：在控制台输入" << endl;
	cout << "f：通过读取文件输入" << endl;
	cout << "请选择：" << endl;
	cin >> option;
	if (option == 'c' || option == 'C')
	{
		int len;
		cout << "\n请输入要编码字符的个数：" << endl;
		cin >> len;
		cout << "请依次输入要编码字符串的二进制（每个字符以分割符|结尾）：" << endl;
		for (int i = 0; i < len; i++)
		{
			cin >> charStream_temp;
			charStream.push_back(charStream_temp);  //将输入的字符流逐个保存到动态数组中
		}
		cout << endl;

		//应该首先建立一个包含所有单个字符ASCII码表的字符串表

		unsigned int init_num = LZW_initDict();//词典初始化后的大小
		/*
		map<string, int>::iterator iter_d;
		for (iter_d = dict.begin(); iter_d != dict.end(); iter_d++)
		{
			cout << iter_d->first << " " << iter_d->second << endl;
		}
		*/
		LZW_compress(init_num);
		vector<unsigned int>::iterator iter;
		cout << "编码后的结果为：" << endl;
		for (iter = codeStream.begin(); iter != codeStream.end(); iter++)
		{
			cout << *iter << " ";
		}
		cout << endl << endl;

		LZW_deCompress(init_num);
		vector<string>::iterator iter_c;
		cout << "解码后的结果为：" << endl;
		for (iter_c = charStream_deCompress.begin(); iter_c != charStream_deCompress.end(); iter_c++)
		{
			cout << *iter_c;
		}
		cout << endl << endl;
	}
	else if (option == 'f' || option == 'F')
	{
		cout << "\n编码后的文件后缀为.lzw" << endl;
		cout << "请输入要编码的文件名：" << endl;
		cin >> filename;
		init_num = comResultWrite(filename);
		double fileSize, fileSize_d;
		fileSize = getFileSize(filename.c_str());
		string compressResultFilename = filename + ".lzwc";
		deComResultWrite(filename, compressResultFilename, init_num);
		fileSize_d = getFileSize(compressResultFilename.c_str());
		cout << endl;
		cout << "压缩前文件大小为：" << fileSize << "bit" << endl;
		cout << "压缩后文件大小为：" << fileSize_d << "bit" << endl;
		cout << "压缩比（压缩前文件大小/压缩后文件大小）为：" << fileSize / fileSize_d << endl;
	}
	else
	{
		cout << "请输入正确选项..." << endl;
	}
	return 0;
}