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
vector<unsigned int> codeStream;  //码字流，用来存储编码生成的码字
vector<char> charStream;  //字符流，存储带解码待解码数据
vector<string> charStream_deCompress;  //解码字符流，存储解码时生成的字符
vector<int> codeToBinDigits;  //用来记录码字每次存储的二进制位数是多少
vector<bool> codeIsTruncated;  //码字是否被截断

void LZW_initDict(unsigned int init_num)
{
	cout << "正在初始化词典..." << endl;
	string s = " ";
	for (int i = 0; i < init_num; i++) {
		s[0] = char(i);
		dict[s] = i;
		reDict[i] = s;
	}
	cout << "词典初始化完成！\n" << endl;
}

void LZW_compress(vector<char> charStream, unsigned int init_num)
{
	cout << "正在编码..." << endl;
	unsigned int code;  //词典中的码字
	string currentPrefix;  //初始时当前前缀P为空
	string currentChar;  //当前字符C
	string str;  //缀-符串S
	map<string, unsigned int>::iterator iter_d;
	vector<char>::iterator iter_c;

	for (iter_c = charStream.begin(); iter_c != charStream.end(); iter_c++)
	{
		currentChar = *iter_c;  //当前字符C:=字符流中的下一个字符
		str = currentPrefix + currentChar;  //缀-符串S=P+C
		iter_d = dict.find(str);

		//判断S是否在词典中
		if (iter_d != dict.end())  //找到了
		{
			currentPrefix = str;  //P:=P+C
		}
		else  //S不在词典中
		{
			iter_d = dict.find(currentPrefix);  //找到代表P的码字
			code = iter_d->second;
			codeStream.push_back(code);  //把码字输出到码字流中
			//cout << "code = " << init_num << "  str = " << str << endl;
			dict.insert(pair<string, int>(str, init_num));  //把缀-符串S添加到词典
			init_num++;  //码值加1
			if (init_num > 4294967295)
			{
				cout << "词典条目超过限制" << endl;
				exit(EXIT_FAILURE);
			}
			currentPrefix = currentChar;  //令P:=C
		}
	}
	//string currentChar_temp(1, currentChar);
	iter_d = dict.find(currentPrefix);  //找到代表P的码字
	code = iter_d->second;
	codeStream.push_back(code);  //把码字输出到码字流中

	/*
	for (iter_d = dict.begin(); iter_d != dict.end(); iter_d++)
	{
		cout << iter_d->first << " " << iter_d->second << endl;
	}
	*/
	cout << "编码完成！" << endl;
	//cout << "当前vector分配的大小: " << codeStream.capacity() << "   " << "当前使用数据的大小: " << codeStream.size() << endl << endl;
}

void LZW_deCompress(long init_num)
{
	//mapunsigned int, string> reDict_2;
	cout << "正在解码..." << endl;
	unsigned int i = 0;
	long code = init_num;  //词典中的码字
	string currentPrefix;  //初始时当前前缀P为空
	char currentChar;  //当前字符C
	unsigned int currentCodeWord;  //当前码字cW
	unsigned int previousCodeWord;  //先前码字cW
	string str;  //缀-符串S
	string currentStr;  //当前缀-符串cS
	string previousStr;  //先前缀-符串pS
	unsigned int codeStream_len = codeStream.size();

	currentCodeWord = codeStream[0];  //当前码字cW:=码字流中的第一个码字
	currentStr = reDict[currentCodeWord];  //保存cW指向的缀-符串S为当前缀-符串cS
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
			//cout << "currentCodeWord = " << currentCodeWord;
			currentStr = reDict[currentCodeWord];  //保存cW指向的缀-符串S为当前缀-符串cS
			//cout << "currentStr = " << currentStr << endl;
			charStream_deCompress.push_back(currentStr);  //输出当前缀-符串到字符流
			//cout << "previousCodeWord = " << previousCodeWord << endl;
			previousStr = reDict[previousCodeWord];  //保存pW指向的缀-符串S为先前缀-符串pS
			//cout << "previousStr = " << previousStr << endl;
			currentPrefix = previousStr;  //P:=pS
			//cout << "currentPrefix = " << currentPrefix << endl;
			currentChar = currentStr[0];  //当前字符C:=当前缀-符串cS的第一个字符
			//cout << "currentChar = " << currentChar << endl;
			str = currentPrefix + currentChar;  //缀-符串S=P+C
			//cout << " code = " << code << "  str = " << str << endl;
			//reDict.insert(pair<unsigned int, string>(code, str));  //把缀-符串S添加到词典
			reDict[code] = str;  //把缀-符串S添加到词典
			/*
			if (code == 273)
			{
				cout << "i: " << i << " currentCodeWord: " << currentCodeWord << " currentPrefix : " << currentPrefix << " currentChar : " << currentChar << endl;
				cout << "currentStr = " << currentStr << endl;
				cout << "reDict[271]: " << reDict[271] << endl;
				cout << "codeStream[146]: " << codeStream[146] << endl;
			}
			*/
			//cout << "code = " << code << endl << endl;
			code++;  //码值加1
		}
		else  //当前缀-符串不在词典中
		{
			//cout << "currentCodeWord = " << currentCodeWord;
			//cout << "previousCodeWord = " << previousCodeWord << endl;
			previousStr = reDict[previousCodeWord];  //保存pW指向的缀-符串S为先前缀-符串pS
			//cout << "previousStr = " << previousStr << endl;
			currentPrefix = previousStr;  //P:=pS
			//cout << "currentPrefix = " << currentPrefix << endl;
			currentChar = reDict[previousCodeWord][0];  //当前字符C:=当前缀-符串cS的第一个字符
			//cout << "currentChar = " << currentChar << endl;
			str = currentPrefix + currentChar;  //缀-符串S=P+C
			charStream_deCompress.push_back(str);  //输出当缀-符串S到字符流
			//cout << " :code = " << code << "  str = " << str << endl;
			//reDict.insert(pair<unsigned int, string>(code, str));  //把缀-符串S添加到词典，此处有BUG，无法正常插入
			reDict[code] = str;  //把缀-符串S添加到词典
			//cout << ":code = " << code << endl << endl;
			code++;  //码值加1
		}
		i++;
	}
	/*
	map<unsigned int, string>::iterator iter_r;
	for (iter_r = reDict.begin(); iter_r != reDict.end(); iter_r++)
	{
		cout << iter_r->first << " " << iter_r->second << endl;
	}
	*/
	cout << "解码完成！" << endl;
}

int numOfBytes(unsigned int num, string &binary)  //获得码字序列的二进制位数
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
	/*
	for (vector<string>::iterator iter = L.begin(); iter != L.end(); iter++)
		cout << *iter;
	cout << endl;
	*/
	/* if (codeToBinDigits % 8 != 0)
	{
		return (codeToBinDigits / 8 + 1);
	}
	else
	{
		return codeToBinDigits / 8;
	} */
}

void fileWR(string filename)
{
	long init_num = 128;  //词典初始化后的大小
	char charStream_temp;
	ifstream inFile;
	//读取存储字符流的文件以便开始编码
	inFile.open(filename.c_str(), ios::binary);  //打开要编码的文件
	if (!inFile.is_open())
	{
		cout << "打开文件“" << filename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		while (inFile.read(&charStream_temp, sizeof(charStream_temp)))  //输出字符流到变量
		{
			charStream.push_back(charStream_temp);
		}
		cout << "\n读取待编码文件成功...\n" << endl;
	}
	inFile.close();
	inFile.clear();
	
	dict.clear();
	reDict.clear();

	LZW_initDict(init_num);
	LZW_compress(charStream, init_num);

	/* unsigned int m = 0;
	vector<unsigned int>::iterator iter;
	cout << "编码后的结果为：" << endl;
	for (iter = codeStream.begin(); iter != codeStream.end(); iter++)
	{
		cout << m << ":" << * iter << " ";
		m++;
	}
	cout << endl << endl; */
	
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
			//cout << hex << codeStream[i] << " " << i << ": " << inFile.gcount() << " " << len << endl;
		}
		cout << "\n读取待解码文件成功...\n" << endl;
	}
	inFile.close();
	inFile.clear();

	LZW_deCompress(init_num);

	char C_word;
	string C_wordTemp;
	unsigned int i;
	int j;
	string deCompressResultFilename = "lzwd_" + filename;  //存储解码结果的文件名加后缀.lzwcd
	//写入解码结果到文件中
	outFile.open(deCompressResultFilename.c_str(), ios:: binary);
	if (!outFile.is_open())
	{
		cout << "打开文件“" << compressResultFilename << "”错误" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		//cout << charStream_deCompress[0] << endl;
		for (i = 0; i < charStream_deCompress.size(); i++)
		{
			C_wordTemp = charStream_deCompress[i];
			for (j = 0; j < C_wordTemp.size(); j++)
			{
				C_word = C_wordTemp[j];
				outFile.write(&C_word, sizeof(C_word));
			}
		}
		cout << "编码结果写入“" << deCompressResultFilename << "”成功" << endl;
	}
	outFile.close();
	outFile.clear();
}

size_t getFileSize(const char* fileName)
{

	if (fileName == NULL)
	{
		cout << "请输入文件名" << endl;
		return -1;
	}
	struct stat statbuf;  // 这是一个存储文件(夹)信息的结构体，其中有文件大小和创建时间、访问时间、修改时间等
	stat(fileName, &statbuf);  // 提供文件名字符串，获得文件属性结构体
	double filesize = statbuf.st_size;// 获取文件大小

	return filesize;
}

int main()
{
	unsigned int init_num = 128;  //词典初始化后的大小
	char option;
	string charStream_temp;  //要编码的字符流
	string filename;
	dict.clear();
	reDict.clear();

	cout << "c：在控制台输入" << endl;
	cout << "f：通过读取文件输入" << endl;
	cout << "请选择：" << endl;
	cin >> option;
	if (option == 'c' || option == 'C')
	{
		cout << "\n请输入要编码的字符串：" << endl;
		cin >> charStream_temp;
		for (int i = 0; i < charStream_temp.length(); i++)
		{
			charStream.push_back(charStream_temp[i]);  //将输入的字符流逐个保存到动态数组中
		}
		cout << endl;

		//应该首先建立一个包含所有单个字符ASCII码表的字符串表
		LZW_initDict(init_num);

		LZW_compress(charStream, init_num);
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
		fileWR(filename);
		double fileSize, fileSize_d;
		fileSize = getFileSize(filename.c_str());
		string filename_d = filename + ".lzwc";
		fileSize_d = getFileSize(filename_d.c_str());
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