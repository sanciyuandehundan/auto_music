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
const string midi_head = "4d 54 68 64";
const string midi_end = "ff 2f 00";
string file_name;
string File_all;
int bpm;//一四分音符几tick
vector<unsigned char> File_char;//以字元为单位存储(byte)
vector<string> File_yingui;//以音轨为单位存储
vector<unsigned char> File_yingui_instruct;//以音轨为单位存储乐器

int main() {
    read("C:\\Users\\a0905\\Downloads\\我不曾忘记-原琴版.mid");
    parse();
}
void read(string path) 
{
    ifstream file("C:\\Users\\a0905\\Downloads\\我不曾忘记-原琴版.mid", std::ios::binary); // 打开 MIDI 文件
    file_name = path;
    char byte; // 用于存储每个字节的临时变量
    
    if (file.is_open()) { // 检查文件是否成功打开
        while (file.read(&byte, sizeof(char))) { // 逐字节读取文件内容
            put_16(byte);//转换为十六进制输出
            File_all += byte;
        }
        file.close(); // 关闭文件
        printf("\n%s容读取成功！\n",path.c_str());
    }
    else {
        printf("无法打开%s文件,请检查路径是否正确\n",file_name.c_str()); // 如果无法打开文件则输出错误信息
    }
    
}

void parse() {
    File_all.erase(0, 8);//删除midi文件头 4d 54 68 64 00 00 00 06
    string time=File_all.substr(4,2);
    //cout << "time";
    //put_16(time);
    File_yingui = split(File_all, midi_end);//将各个音轨分离储存
    for (string& yg : File_yingui) {//yg为引用，可以影响File_yingui
        yg.erase(yg.begin(), yg.begin() + 4);//去除音轨头
    }
    /*for (char a : File_all)
    {
        File_char.push_back(a);
    }
    File_char.erase(File_char.begin(), File_char.begin() + 8);//删除midi文件头 4d 54 68 64 00 00 00 06
    stringstream ss(File_char);
    for(string temp:File_ch)*/
}

vector<string> split(const string &data,const string &anchor) {
    vector<string> temp;
    size_t start = 0, end = 0;
    while ((end = data.find(anchor)) != string::npos) {//end是anchor的第一个字元的索引
        temp.push_back(data.substr(start, end - start));//从start到end-1
        start = end + anchor.length();
    }
    //midi文件的音轨是 内容结尾 内容结尾 的，所以不需要处理最后一个音轨后的内容
    return temp;
}//切割成不同音轨，没有结尾

void put_16(char &c){
    cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";//转换为十六进制输出
}//以16进制输出(字元)
void put_16(string &s) {
    for (char c : s) {
        cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";//转换为十六进制输出
    }
}//以16进制输出(字串)

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
