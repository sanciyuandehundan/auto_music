// auto_music.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;

void read(string path);
void parse();
vector<string> split(const string& data, const string& anchor);
void put_16(unsigned char &c);
void put_16(string &s);
void put_16(int &i);
int trans_16s_10int(string a);
class Yingui;
//提前宣告

int midi_head_long = 0;//文件头多长
const string midi_head = "\x4d\x54\x68\x64";//文件头
const string yingui_head = "\x4d\x54\x72\x6b";//文件头
const string midi_end = "\xff\x2f\x00";//音轨结尾
const string midi_speed = "\xff\x51\x03";//速度事件开头
const string midi_pai = "\xff\x58\x04";//拍子事件开头
const string midi_diaohao = "\xff\x59\x02";//调号事件开头
int midi_note_long = 500;//一个四分音符多长(微秒)
string file_name;//文件名
string File_all;//这个文件，使用时尽量以引用为主，减少不必要复制
vector<string> File_yingui;//以音轨为单位存储
vector<unsigned char> File_yingui_instruct;//以音轨为单位存储乐器
unsigned short File_yingui_class;//该文件音轨类型，0000单音轨，0001同步多音轨，0002异步多音轨(不受理)
unsigned short File_yingui_num;//该文件音轨数量,全局音轨计算在内
unsigned short File_time_tick;//该文件一个四分音符几tick
string tempo;//记录节拍
char bell;//节拍器时钟
char four_32;//一个四分音符包含几个三十二音符
vector<Yingui*> yingui;//音轨
//全局变量

class Yingui {
public:

    class play {
    public:
        play* next = nullptr;
        virtual void play_() {
            next->play_();
        }
    };
    class play_down :public play {
    public:
        int note;
        void play_() {
            next->play_();
        }
        play_down(play* previous, int note_) {
            note = note_;
            previous->next = this;
            put_16(note);
            cout << endl;
        }
    };
    class play_up :public play {
    public:
        int note;
        void play_() {
            next->play_();
        }
        play_up(play* previous, int note_) {
            note = note_;
            previous->next = this;
            put_16(note);
            cout << endl;
        }
    };
    class play_stop :public play {
    public:
        int time;
        void play_() {
            next->play_();
        }
        play_stop(play* previous,int time_) {
            time = time_;
            previous->next = this;
            put_16(time);
            cout << endl;
        }
    };
    class play_end :public play {
    public:
        void play_() {
            cout << "演奏结束";
        }
        play_end(play* previous) {
            previous->next = this;
        }
    };
    struct event_
    {
    public:
        enum event_type {
            _8x = 0//松开按键
            , _9x = 1//按下按键
            , _Ax = 2//触后音符
            , _Bx = 3//控制器
            , _Cx = 4//改变乐器
            , _Dx = 5//触后通道
            , _Ex = 6//滑音
            , _F0 = 7//系统码
            , _FF = 8//其他格式，后有额外标识
            , null = 9//未决定
            , stop = 10//间隔时间
        };
        enum event_type_FF {
            _01 = 0//文本信息，歌曲备注
            , _02 = 1//版权信息
            , _03 = 2//歌曲或音轨名称
            , _04 = 3//乐器名称
            , _05 = 4//歌词
            , _06 = 5//文本标记
            , _07 = 6//开始点
            , _2F = 7//音轨结束
            , _51 = 8//速度
            , _58 = 9//节拍
            , _59 = 10//调号
            , _7F = 11//音序特定信息
        };
    };

    int stop_handle(unsigned char c) {
        int temp = 0;
        while (true) {
            put_16(c);
            if (c & 0x80) {
                temp = (temp << 7) + (c & 0b01111111);
                c = static_cast<unsigned char>(stre.get());
            }
            else {
                temp = (temp << 7) + c;
                break;
            }
        }
        cout << endl;
        return temp;
    }
    void other_handle(unsigned char ty,unsigned char len) {
        switch (ty) {
        case 0x01:
            cout << "备注" << endl;
            text.clear();
            text.resize(len);
            stre.read(&text[0], len);//读取备注
            cout << text << endl;
            break;
        case 0x02:
            cout << "版权" << endl;
            power.clear();
            power.resize(len);
            stre.read(&power[0], len);//读取版权信息
            cout << power << endl;
            break;
        case 0x03:
            cout << "音轨名" << endl;
            name.clear();
            name.resize(len);
            stre.read(&name[0], len);//读取音轨名
            cout << name << endl;
            break;
        case 0x04:
            cout << "乐器名称" << endl;
            instruct_name.clear();
            instruct_name.resize(len);
            stre.read(&instruct_name[0], len);//读取乐器名
            cout << instruct_name << endl;
            break;
        case 0x2f: {//音轨结束
            cout << "结束" << endl;
            play_end* e =new play_end(last);
            last = e;
            have = false;
            break;
        }
        case 0x51: {
            cout << "音符长度" << endl;
            midi_note_long = 0;
            unsigned char c;
            for (int i = 0; i < 3; i++) {
                c = static_cast<unsigned char>(stre.get());
                midi_note_long = (midi_note_long << 8) + c;
                put_16(c);
            }
            cout <<midi_note_long<< endl;
            break;
        }
        case 0x58:
            cout << "节拍" << endl;
            unsigned char h, j, k, l;
            h=static_cast<unsigned char>(stre.get());//拍子分子
            j= static_cast<unsigned char>(stre.get());//拍子分母
            k= static_cast<unsigned char>(stre.get());//节拍器时钟(未知)
            l= static_cast<unsigned char>(stre.get());//一个四分音符包含几个三十二分音符
            h += '0';
            j = (2 ^ j) + '0';
            k += '0';
            l += '0';
            tempo += h + '/' + j;
            break;
        case 0x59:
            cout << "调号" << endl;
            unsigned char ud, bs;
            ud= static_cast<unsigned char>(stre.get());//获取升降号数
            bs= static_cast<unsigned char>(stre.get());//获取大小调
            break;
        }
    }

    string data;//原始音轨字串
    string name="无名";//音轨名
    string text;//音轨备注
    string power;//版权信息
    string instruct_name;//乐器名称
    unsigned char instruct=0;//乐器，默认钢琴
    stringstream stre;//字串流
    int load;//通道
    play start = play();
    play *last = &start;
    bool have = true;

    Yingui(string data_) {
        const unsigned char FF = '\xff';
        const unsigned char F0 = '\xf0';
        data = data_;
        stre = stringstream(data);//将字串输入流
        stre.ignore(4);//跳过开头标识长度的内容
        unsigned char c;//暂存字元
        event_::event_type temp_previous = event_::event_type::null;//暂存上一个读取事件的类型
        while (have) {
            cout << "__________________________" << endl;
            c = static_cast<unsigned char>(stre.get());
            cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";
            if (temp_previous == event_::stop) {//如果上一个是间隔事件，此事件可能是间隔事件以外的事件
                cout << "test ";
                switch (c >> 4)
                {
                case 0x8: {//松开事件
                    cout << "松开";
                    play_up* u=new play_up(last, stre.get());//创建
                    last = u;//记录指令链最后一个
                    stre.ignore(1);//忽略力度符号
                    temp_previous = event_::_8x;
                    break;
                }
                case 0x9: {//按下事件
                    cout << "按下";
                    play_down* d=new play_down(last, stre.get());//创建
                    last = d;//记录指令链最后一个
                    stre.ignore(1);//忽略力度符号
                    temp_previous = event_::_9x;
                    break;
                }
                case 0xc://设定乐器
                    cout << "设定乐器"<<endl;
                    instruct = stre.get();
                    temp_previous = event_::_Cx;
                    break;
                case 0xf://f开头事件
                    cout << "f开头事件 ";
                    if (c == FF) {
                        cout << "元事件 ";
                        unsigned char ty, len;
                        ty= static_cast<unsigned char>(stre.get());//获取事件类型
                        len= static_cast<unsigned char>(stre.get());//获取参数长度
                        other_handle(ty, len);
                        temp_previous = event_::_FF;
                    }//元事件
                    else {
                        cout << "系统码";
                        stre.ignore(2);
                        temp_previous = event_::_F0;
                    }//系统码
                    break;
                case 0xa:
                    cout << "无法呈现a";
                    stre.ignore(2);
                    temp_previous = event_::_Ax;
                    break;//原琴无法呈现这个
                case 0xb:
                    cout << "无法呈现b";
                    stre.ignore(2);
                    temp_previous = event_::_Bx;
                    break;//原琴无法呈现这个
                case 0xd:
                    cout << "无法呈现d";
                    stre.ignore(2);
                    temp_previous = event_::_Dx;
                    break;//原琴无法呈现这个
                case 0xe:
                    cout << "无法呈现e";
                    stre.ignore(2);
                    temp_previous = event_::_Ex;
                    break;//原琴无法呈现这个
                default: {//仍是间隔事件
                    cout << "间隔";
                    play_stop* s =new play_stop(last, stop_handle(c));
                    last = s;
                    temp_previous = event_::stop;
                    break;
                }
                }
            }
            else {//如果上一个事件不是间隔事件，那此事件必为间隔事件
                cout << "间隔";
                play_stop* s =new play_stop(last, stop_handle(c));
                last = s;
                temp_previous = event_::stop;
            }
        }
    }

};

int main() {
    read("C:\\Users\\a0905\\Downloads\\spiral.mid");
    //read("C:\\Users\\a0905\\Downloads\\「spiral」- 无职转生S2 OP-V1.mid");
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
        main();
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
    File_yingui.erase(File_yingui.begin());//删除不是音轨的部分
    cout << "\n__________________________"<<endl;
    for (string& yg : File_yingui) {//yg为引用，可以影响File_yingui
        put_16(yg);
        cout << "\n__________________________" <<endl;
        Yingui* y=new Yingui(yg);
        yingui.push_back(y);
    }
}

vector<string> split(const string& data, const string& anchor) {
    vector<string> temp;
    size_t start = 0, end = 0;
    while ((end = data.find(anchor, start)) != string::npos) {
        temp.push_back(data.substr(start, end - start));
        start = end + anchor.length();
    }
    // 处理字符串以锚点结尾的情况
    if (start < data.length()) {
        temp.push_back(data.substr(start));
    }
    return temp;
}



void put_16(unsigned char &c){
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