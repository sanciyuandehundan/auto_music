// auto_music.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

void read(string path);
void parse();
vector<string> split(const string& data, const string& anchor);
void put_16(char &c);
void put_16(string &s);
void put_16(int &i);
int trans_16s_10int(string a);
//方法提前宣告

int midi_head_long = 0;//文件头多长
const string midi_head = "\x4d\x54\x68\x64";//文件头
const string yingui_head = "\x4d\x54\x72\x6b";//文件头
const string midi_end = "\xff\x2f\x00";//音轨结尾 ff 2f 00
const string midi_speed = "\xff\x51\x03";//速度事件开头
const string midi_pai = "\xff\x58\x04";//拍子事件开头
const string midi_diaohao = "\xff\x59\x02";//调号事件开头
const int midi_note_long = 500;//一个四分音符默认多长(毫秒)
string file_name;//文件名
string File_all;//这个文件，使用时尽量以引用为主，减少不必要复制
vector<string> File_yingui;//以音轨为单位存储
vector<unsigned char> File_yingui_instruct;//以音轨为单位存储乐器
unsigned short File_yingui_class;//该文件音轨类型，0000单音轨，0001同步多音轨，0002异步多音轨(不受理)
unsigned short File_yingui_num;//该文件音轨数量,全局音轨计算在内
unsigned short File_time_tick;//该文件一个四分音符几tick
//全局变量

struct Yingui {
public:
    string data;//原始音轨字串
    string name;//音轨名
    unsigned char instruct;


    Yingui(string complete) {

    }
};

int main() {
    read("C:\\Users\\a0905\\Downloads\\「spiral」- 无职转生S2 OP-V1.mid");
    parse();
}

void read(string path) 
{
    ifstream file(path, std::ios::binary); // 打开 MIDI 文件
    
    file_name = path.substr(path.find_last_of("\\") + 1);//去除父文件夹路径
    file_name = file_name.substr(0, file_name.find_last_of("."));//去除文件扩展名

    char byte; // 用于存储每个字节的临时变量
    if (file.is_open()) { // 检查文件是否成功打开
        while (file.read(&byte, sizeof(char))) { // 逐字节读取文件内容
            //put_16(byte);//转换为十六进制输出
            File_all += byte;
        }
        file.close(); // 关闭文件
        printf("\n%s读取成功！\n",file_name.c_str());
    }
    else {
        printf("无法打开%s文件,请检查路径是否正确\n",file_name.c_str()); // 如果无法打开文件则输出错误信息
    }
    
}
//读取文件到运行内存

void parse() {
    midi_head_long = trans_16s_10int(File_all.substr(4, 4));//记录接下来几个字元是文件头
    if (midi_head_long != 6) {
        printf("文件格式非法,请选择其他文件");
        main();
    }
    File_yingui_class = trans_16s_10int(File_all.substr(8, 2));//读取音轨类型
    if (File_yingui_class == 2) {
        printf("文件格式非法,请选择其他文件");
        main();
    }
    File_yingui_num = trans_16s_10int(File_all.substr(10,2));//读取音轨数量



    File_yingui = split(File_all, yingui_head);//将各个音轨分离储存
    cout << "\n__________________________"<<endl;
    for (string& yg : File_yingui) {//yg为引用，可以影响File_yingui
        put_16(yg);
        cout << "\n__________________________" <<endl;
    }
}

vector<string> split(const string& data, const string& anchor) {
    vector<string> temp;
    size_t start = 0, end = 0;
    while ((end = data.find(anchor, start)) != string::npos) {
        // 处理连续出现多个锚点的情况
        if (start == end) {
            temp.push_back("");
        }
        else {
            temp.push_back(data.substr(start, end - start));
        }
        start = end + anchor.length();
    }
    // 处理字符串以锚点结尾的情况
    if (start < data.length()) {
        temp.push_back(data.substr(start));
    }
    return temp;
}



void put_16(char &c){
    cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";//转换为十六进制输出
}
//以16进制输出(字元)
void put_16(string &s) {
    for (char c : s) {
        cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";//转换为十六进制输出
    }
}
//以16进制输出(字串)
void put_16(int& i) {
    cout << std::hex << i << " ";
}
//以16进制输出(整数)

int trans_16s_10int(string s) {
    int re = 0;
    int s_long=s.length();
    for (char c : s) {
        re = (re << 8) + c;
    }
    return re;
}
//将以字串形式存储的16进制整数转换成以int存储的10进制整数

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
/*
vector<string> split(const string &data,const string &anchor) {
    vector<string> temp;
    string k;
    size_t start = 0, end = 0;
    while ((end = data.find(anchor)) != string::npos) {//end是anchor的第一个字元的索引
        k = data.substr(start, end - start);
        put_16(k);
        temp.push_back(k);//从start到end-1
        start = end + anchor.length();
    }
    //midi文件的音轨是 内容结尾 内容结尾 的，所以不需要处理最后一个音轨后的内容
    return temp;
}
//切割成不同音轨，有结尾
*/