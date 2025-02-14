﻿// auto_music.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <winuser.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include <direct.h>
using namespace std;

class txttomid;
void read_mid(string path);
void parse();
vector<string> split(const string& data, const string& anchor);
void put_16(unsigned char &c);
void put_16(string &s);
void put_16(int &i);
int trans_16s_10int(string a);
void time_handle();
class Yingui;
void play_th(Yingui* yg);
void creat_th(int* choice, int num);
void allclear();
void yinguiclear();
void threadclear();
void choice_file();
void choice_yingui();
void startplay();
enum key {
    Q = 0x48,
    W = 0x4a,
    E = 0x4c,
    R = 0x4d,
    T = 0x4f,
    Y = 0x51,
    U = 0x53,
    A = 0x3c,
    S = 0x3e,
    D = 0x40,
    F = 0x41,
    G = 0x43,
    H = 0x45,
    J = 0x47,
    Z = 0x30,
    X = 0x32,
    C = 0x34,
    V = 0x35,
    B = 0x37,
    N = 0x39,
    M = 0x3b
};
//提前声明

int head = 0;//文件头多长
const string midi_head = "\x4d\x54\x68\x64";//文件头
const string yingui_head = "\x4d\x54\x72\x6b";//音轨头
const string midi_end = "\xff\x2f\x00";//音轨结尾
const string midi_speed = "\xff\x51\x03";//速度事件开头
const string midi_pai = "\xff\x58\x04";//拍子事件开头
const string midi_diaohao = "\xff\x59\x02";//调号事件开头
int note_weimiao = 500000;//一个四分音符多长(微秒)
string file_name;//文件名
string all;//这个文件，使用时尽量以引用为主，减少不必要复制
vector<string> yingui_string;//以音轨为单位存储
unsigned short yingui_class;//该文件音轨类型，0000单音轨，0001同步多音轨，0002异步多音轨(不受理)
unsigned short yingui_num;//该文件音轨数量,全局音轨计算在内
unsigned short time_tick;//该文件一个四分音符几tick
unsigned short ud;//调号升降号数
unsigned short bs;//大小调
double tick_weimiao;//一个tick几微妙
string tempo;//记录节拍
char bell;//节拍器时钟
char four_32;//一个四分音符包含几个三十二音符
vector<Yingui*> yingui;//音轨
vector<thread> threads;//播放执行绪
mutex mtx;//互斥锁
condition_variable cv;
bool thread_start = false;
bool creatermode = true;//开发者模式
int* choice = new int[yingui_num];//选择哪几个
int choice_num = -999;//选择几个
vector<unsigned char> temp_key;
int maxtime = 0;
//全局变量

class txttomid {
public:
    struct note {
        //txttomid* parent;
    private:
        const static int timebase = 1920;//一分音符几tick
    public:
        bool timeevent = false;
        int time = 0;
        int endtimeadd = 0;
        note* previous = nullptr;
        note* next = nullptr;
        int num = 0;
        unsigned char key[7] = { 0 };
        note(vector<unsigned char> key_) {
            //parent = pa;
            for (int i = 0; i < key_.size(); i++) {
                key[i] = key_[i];
            }
            num = key_.size();
            temp_key.clear();

            if (key_[0] != 0x1) {
                if (timeevent)cout << "time";
                if(creatermode)cout << "num: " << num << '|';
                for (int i = 0; i < num; i++) {
                    if (creatermode)cout << hex << unsigned int(key_[i]) << ' ';
                }
                if (creatermode)cout << endl;
            }
            else {
                if (creatermode)cout << "creattime" << endl;
                timeevent = true;
            }
        }
        void timeset(int t) {
            time = timebase / t;
        }
        void timeadd(note* n) {
            endtimeadd += n->time;
        }
        void nextset(note* ne) {
            if (!ne->timeevent) {
                next = ne;
                ne->previous = this;
            }
            else {
                timeadd(ne);
            }
        }
        void push(vector<unsigned char>* yg) {
            /*if (timeevent)cout << "time";
            for (int i = 0; i < num; i++) {
                cout <<hex<< unsigned int(key[i]) << ' ';
            }
            cout << endl;*/
            for (int i = 0; i < num; i++) {
                yg->push_back(0x90);
                yg->push_back(key[i]);
                yg->push_back(0x50);
                if (i != num - 1)yg->push_back(0x0);
            }
            int tq = time * 0.9;
            if (tq > 0b111111111111111111111) {
                yg->push_back(0b10000000 + (((tq >> 21)) & 0b01111111));
            }
            if (tq > 0b11111111111111) {
                yg->push_back(0b10000000 + (((tq >> 14)) & 0b01111111));
            }
            if (tq > 0b1111111) {
                yg->push_back(0b10000000 + (((tq >> 7)) & 0b01111111));
            }
            yg->push_back(tq & 0b01111111);
            //yg->push_back(0x83);
            //yg->push_back(0x30);
            for (int i = 0; i < num; i++) {
                yg->push_back(0x80);
                yg->push_back(key[i]);
                yg->push_back(0x0);
                if (i != num - 1)yg->push_back(0x0);
            }
            tq = time - tq + endtimeadd;
            if (tq > 0b111111111111111111111) {
                yg->push_back(0b10000000 + (((tq >> 21)) & 0b01111111));
            }
            if (tq > 0b11111111111111) {
                yg->push_back(0b10000000 + (((tq >> 14)) & 0b01111111));
            }
            if (tq > 0b1111111) {
                yg->push_back(0b10000000 + (((tq >> 7)) & 0b01111111));
            }
            yg->push_back(tq & 0b01111111);
            //yg->push_back(0x30);
        }
    };
    string all;
    string file_name;
    vector<unsigned char> yingui;
    note* nstart = nullptr;
    note* nend = nullptr;
    vector<note*> temp_note;
    unsigned int bpm = 120;

    void read_txt(string path)
    {
        ifstream file(path, std::ios::binary); // 打开 MIDI 文件

        file_name = path.substr(path.find_last_of("\\") + 1);//去除父文件夹路径
        file_name = file_name.substr(0, file_name.find_last_of("."));//去除文件扩展名
        //file >> noskipws;

        char byte; // 用于存储每个字节的临时变量
        if (file.is_open()) { // 检查文件是否成功打开
            while (file.read(&byte, sizeof(char))) { // 逐字节读取文件内容
                if (byte != '\n' && byte != '\r') {
                    all += byte;
                }
            }
            file.close(); // 关闭文件
            printf("\n%s读取成功！\n__________________________\n", file_name.c_str());
        }
        else {
            printf("__________________________\n无法打开%s文件,请检查路径是否正确\n", file_name.c_str()); // 如果无法打开文件则输出错误信息
        }
        if (creatermode)cout << all << endl;
    }

    /*void push(note* n) {

        for (int i = 0; i < temp_key.size(); i++) {
            yingui.push_back(0x90);
            yingui.push_back(temp_key[i]);
            yingui.push_back(0x50);
            if (i != temp_key.size() - 1)yingui.push_back(0x0);
        }
        yingui.push_back(0x83);
        yingui.push_back(0x30);
        for (int i = 0; i < temp_key.size(); i++) {
            yingui.push_back(0x80);
            yingui.push_back(temp_key[i]);
            yingui.push_back(0x0);
            if (i != temp_key.size() - 1)yingui.push_back(0x0);
        }
        yingui.push_back(0x30);
        temp_key.clear();
    }*/

    int transform()
    {

        string path;
        cout << "文件路径: ";
        cin >> path;
        cout << "bpm: ";
        cin >> bpm;
        bpm *= 4;
        read_txt(path);
        ofstream file((file_name + ".mid"), std::ios::binary);

        // 检查文件是否成功打开
        if (!file.is_open()) {
            std::cerr << "无法打开文件：" << file_name << endl;
            return 1;
        }

        vector<unsigned int> yushuru = { 0x4d,0x54,0x68,0x64,0x0,0x0,0x0,0x6,0x0,0x01,0x0,0x2,
            (bpm >> 8) & 0b11111111,bpm & 0b11111111,0x4d ,0x54 ,0x72 ,0x6b ,0x0 ,0x0 ,0x0 ,0xb ,0x0 ,0xff,0x51,0x03,0x07,0x81,0x1b,0x0,0xff ,0x2f ,0x0 ,
            0x4d ,0x54 ,0x72 ,0x6b };

        for (int i : yushuru) {
            file << char(i);
        }

        yingui.push_back(0x0);
        yingui.push_back(0xc0);
        yingui.push_back(0x0);
        yingui.push_back(0x0);
        bool input_key = true;
        bool input_note = true;
        for (char c : all) {
            if (creatermode)cout << dec << "c: " << c << '|' << int(c) << endl;
            switch (c) {
            case 'Q': temp_key.push_back(0x48); break;
            case 'W': temp_key.push_back(0x4a); break;
            case 'E': temp_key.push_back(0x4c); break;
            case 'R': temp_key.push_back(0x4d); break;
            case 'T': temp_key.push_back(0x4f); break;
            case 'Y': temp_key.push_back(0x51); break;
            case 'U': temp_key.push_back(0x53); break;
            case 'A': temp_key.push_back(0x3c); break;
            case 'S': temp_key.push_back(0x3e); break;
            case 'D': temp_key.push_back(0x40); break;
            case 'F': temp_key.push_back(0x41); break;
            case 'G': temp_key.push_back(0x43); break;
            case 'H': temp_key.push_back(0x45); break;
            case 'J': temp_key.push_back(0x47); break;
            case 'Z': temp_key.push_back(0x30); break;
            case 'X': temp_key.push_back(0x32); break;
            case 'C': temp_key.push_back(0x34); break;
            case 'V': temp_key.push_back(0x35); break;//kk
            case 'B': temp_key.push_back(0x37); break;
            case 'N': temp_key.push_back(0x39); break;
            case 'M': temp_key.push_back(0x3b); break;//kk
            case '(':
                input_key = false;
                break;
            case ')':
                input_key = true;
                break;
            case'/':
                for (note* n : temp_note) {
                    n->timeset(4 * temp_note.size());
                    if (nstart == nullptr) {
                        nstart = n;
                        nend = n;
                    }
                    else {
                        nend->nextset(n);
                        if (!n->timeevent) {
                            nend = n;
                        }
                    }
                }
                temp_note.clear();
                break;
            case ' ':
                temp_key.push_back(0x1);
                break;
            default:
                if (creatermode)cout << "error: " << c << endl;
                break;
            }
            if (input_key && temp_key.size() != 0) {
                //cout << "test1"<<endl;
                temp_note.push_back(new note(temp_key));
                //cout << "test2" << endl;
            }
            //push(input);
        }

        note* t = nstart;

        while (t != nullptr) {
            t->push(&yingui);
            t = t->next;
        }

        yingui.push_back(0xff);
        yingui.push_back(0x2f);
        yingui.push_back(0x0);

        file << char((yingui.size() >> 24) & 0b11111111);
        file << char((yingui.size() >> 16) & 0b11111111);
        file << char((yingui.size() >> 8) & 0b11111111);
        file << char(yingui.size() & 0b11111111);

        for (int i : yingui)
        {
            file << char(i);
            if (creatermode)cout << hex << i << " ";
        }
        file.close();
        cout << "\n长度： " << yingui.size() << endl;
        cout << file_name << "创建完成";
    }

};

class Yingui {
public:
    class play;
    class play_down;
    class play_up;
    class play_stop;
    string data;//原始音轨字串
    string name = "无名";//音轨名
    string text = "无";//音轨备注
    string power = "无";//版权信息
    string instruct_name = "无";//乐器名称
    unsigned int instruct = 0;//乐器，默认钢琴
    stringstream stre;//字串流
    bool have = true;
    int alltime;//总共几微妙

    class play {
    public:
        bool down = false;
        Yingui* parent;
        play* next = nullptr;
        play* previous = nullptr;
        //char k[7];
        virtual void handle_again() {
            next->handle_again();
        }
        virtual void play_() {
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [] { return thread_start; }); // 等待条件变量满足
            }
            next->play_();
        }
        virtual void down_add(play_down* pd) {

        }
    };
    class play_down :public play {
    public:
        int note;
        //vector<char> k;
        //char k;
        WORD k[7];
        int k_num = 0;
        INPUT input_d[7] = { 0 };
        INPUT input_u[7] = { 0 };
        void handle_again() {
            for (int i = 0; i < 7; i++) {
                input_d[i].type = INPUT_KEYBOARD;
                input_d[i].ki.wScan = 0;
                input_d[i].ki.dwFlags = 0;
                input_d[i].ki.dwExtraInfo = 0;
                input_u[i].type = INPUT_KEYBOARD;
                input_u[i].ki.wScan = 0;
                input_u[i].ki.dwFlags = KEYEVENTF_KEYUP;
                input_u[i].ki.dwExtraInfo = 0;
            }
            switch (note) {
                /*case 0x48:k.push_back('Q'); break;
                case 0x4a:k.push_back('W'); break;
                case 0x4c:k.push_back('E'); break;
                case 0x4d:k.push_back('R'); break;
                case 0x4f:k.push_back('T'); break;
                case 0x51:k.push_back('Y'); break;
                case 0x53:k.push_back('U'); break;
                case 0x3c:k.push_back('A'); break;
                case 0x3e:k.push_back('S'); break;
                case 0x40:k.push_back('D'); break;
                case 0x41:k.push_back('F'); break;
                case 0x43:k.push_back('G'); break;
                case 0x45:k.push_back('H'); break;
                case 0x47:k.push_back('J'); break;
                case 0x30:k.push_back('Z'); break;
                case 0x32:k.push_back('X'); break;
                case 0x34:k.push_back('C'); break;
                case 0x35:k.push_back('V'); break;
                case 0x37:k.push_back('B'); break;
                case 0x39:k.push_back('N'); break;
                case 0x3b:k.push_back('M'); break;
                default:
                    k.push_back(VK_SPACE);
                    break;
                }*/
            case 0x48:k[k_num] = 0x51; break;
            case 0x4a:k[k_num] = 0x57; break;
            case 0x4c:k[k_num] = 0x45; break;
            case 0x4d:k[k_num] = 0x52; break;
            case 0x4f:k[k_num] = 0x54; break;
            case 0x51:k[k_num] = 0x59; break;
            case 0x53:k[k_num] = 0x55; break;
            case 0x3c:k[k_num] = 0x41; break;
            case 0x3e:k[k_num] = 0x53; break;
            case 0x40:k[k_num] = 0x44; break;
            case 0x41:k[k_num] = 0x46; break;
            case 0x43:k[k_num] = 0x47; break;
            case 0x45:k[k_num] = 0x48; break;
            case 0x47:k[k_num] = 0x4A; break;
            case 0x30:k[k_num] = 0x5A; break;
            case 0x32:k[k_num] = 0x58; break;
            case 0x34:k[k_num] = 0x43; break;
            case 0x35:k[k_num] = 0x56; break;
            case 0x37:k[k_num] = 0x42; break;
            case 0x39:k[k_num] = 0x4E; break;
            case 0x3b:k[k_num] = 0x4D; break;
            default:
                k[k_num] = VK_SPACE;
                break;
            }
            k_num = 1;
            input_d[0].ki.wVk = k[0];
            input_u[0].ki.wVk = k[0];

            if (creatermode) cout << "按下: " << k[0] << '|' << k[1] << '|' << k[2] << '|' << k[3] << '|' << k[4] << '|' << k[5] << '|' << k[6] << '|';//.data();
            if (creatermode)cout << "\n__________________________" << endl;
            if (previous->down)previous->down_add(this);
            //DELETE &k;
            next->handle_again();
        }
        void play_() {
            /*for (char t : k) {
                keybd_event(t, 0, 0, 0);
            }
            for (char t : k) {
                keybd_event(t, 0, KEYEVENTF_KEYUP, 0);
            }*/
            /*for (INPUT k : input_d) {
                cout << k.ki.wVk << endl;
            }*/
            SendInput(k_num, &input_d[0], sizeof(INPUT));
            SendInput(k_num, &input_u[0], sizeof(INPUT));
            next->play_();
        }
        play_down(play* previous_, int note_, Yingui* pare) {
            down = true;
            note = note_;
            previous_->next = this;
            previous = previous_;
            parent = pare;
            if (creatermode)put_16(note);
            //if (creatermode)cout << endl;
        }
        void down_add(play_down* pd) {
            k[k_num] = pd->k[0];
            input_d[k_num].ki.wVk = k[k_num];
            input_u[k_num].ki.wVk = k[k_num];
            k_num += 1;
            if(creatermode)cout << k_num << endl;

            next = pd->next;
            pd->next->previous = this;
        }
    };

    class play_up :public play {
    public:
        int note;
        char k=' ';
        void handle_again() {
            /*switch (note) {
            case 0x48:k = 'Q'; break;
            case 0x4a:k = 'W'; break;
            case 0x4c:k = 'E'; break;
            case 0x4d:k = 'R'; break;
            case 0x4f:k = 'T'; break;
            case 0x51:k = 'Y'; break;
            case 0x53:k = 'U'; break;
            case 0x3c:k = 'A'; break;
            case 0x3e:k = 'S'; break;
            case 0x40:k = 'D'; break;
            case 0x41:k = 'F'; break;
            case 0x43:k = 'G'; break;
            case 0x45:k = 'H'; break;
            case 0x47:k = 'J'; break;
            case 0x30:k = 'Z'; break;
            case 0x32:k = 'X'; break;
            case 0x34:k = 'C'; break;
            case 0x35:k = 'V'; break;//kk
            case 0x37:k = 'B'; break;
            case 0x39:k = 'N'; break;
            case 0x3b:k = 'M'; break;//kk
            default:
                k = VK_SPACE;
                if (creatermode)cout << "无法映射" << note<<endl;
                break;
            }*/
            //if (creatermode)cout << "松开: " << k;
            //if (creatermode)cout << "\n__________________________" << endl;
            previous->next = next;
            next->handle_again();
        }
        void play_() {
            next->play_();
        }
        play_up(play* previous_, int note_,Yingui* pare) {
            note = note_;
            parent = pare;
            previous_->next = this;
            previous = previous_;
            
            //put_16(note);
            //cout << endl;
        }
    };
    

    class play_stop :public play {
    public:
        int tick;
        int time = 0;
        void handle_again() {
            time = static_cast<double>(tick) * tick_weimiao;
            if (creatermode)cout << dec << "微秒: " << time << " | tick: ";
            if (creatermode)put_16(tick);
            if (creatermode)cout << "\n__________________________" << endl;
            parent->alltime += time;
            if (time == 0) {
                previous->next = next;
                next->previous = previous;
            }
            next->handle_again();
        }
        void play_() {
            this_thread::sleep_for(chrono::microseconds(time));
            next->play_();
        }
        play_stop(play* previous_, int time_, Yingui* pare) {
            time = 0;
            tick = time_;
            parent = pare;
            previous_->next = this;
            previous = previous_;
            //put_16(tick);
            //cout << endl;
        }
    };
    class play_end :public play {
    public:
        void handle_again() {
            if (creatermode)cout << "处理结束";
            if (creatermode)cout << "\n__________________________" << endl;
        }
        void play_() {
            cout << "演奏结束"<<endl;
        }
        play_end(play* previous_,Yingui* pare) {
            previous_->next = this;
            previous = previous_;
            parent = pare;
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

    play start = play();
    play* last = &start;

    int stop_handle(unsigned char c) {
        int temp = 0;
        while (true) {
            //put_16(c);
            if (c & 0x80) {
                temp = (temp << 7) + (c & 0b01111111);
                c = static_cast<unsigned char>(stre.get());
            }
            else {
                temp = (temp << 7) + c;
                break;
            }
        }
        //cout << endl;
        return temp;
    }
    void other_handle(unsigned char ty,unsigned char len) {
        switch (ty) {
        case 0x01://读取备注
            if (creatermode)cout << "备注" << endl;
            text.clear();
            text.resize(len);
            stre.read(&text[0], len);
            if (creatermode)cout << text << endl;
            break;
        case 0x02://读取版权信息
            if (creatermode)cout << "版权" << endl;
            power.clear();
            power.resize(len);
            stre.read(&power[0], len);
            if (creatermode)cout << power << endl;
            break;
        case 0x03://读取音轨名
            if (creatermode)cout << "音轨名" << endl;
            name.clear();
            name.resize(len);
            stre.read(&name[0], len);
            if (creatermode)cout << name << endl;
            break;
        case 0x04://读取乐器名
            if (creatermode)cout << "乐器名称" << endl;
            instruct_name.clear();
            instruct_name.resize(len);
            stre.read(&instruct_name[0], len);
            if (creatermode)cout << instruct_name << endl;
            break;
        case 0x2f: {//音轨结束
            if (creatermode)cout << "结束" << endl;
            play_end* e =new play_end(last,this);
            last = e;
            have = false;
            break;
        }
        case 0x51: {//读取一个四分音符多长
            if (creatermode)cout << "音符长度" << endl;
            note_weimiao = 0;
            unsigned char c;
            for (int i = 0; i < 3; i++) {
                c = static_cast<unsigned char>(stre.get());
                note_weimiao = (note_weimiao << 8) + c;
                if (creatermode)put_16(c);
            }
            if (creatermode)cout <<note_weimiao<< endl;
            break;
        }
        case 0x58://读取节拍
            if (creatermode)cout << "节拍" << endl;
            unsigned char h, j, k, l;
            h=static_cast<unsigned char>(stre.get());//拍子分子
            j= static_cast<unsigned char>(stre.get());//拍子分母
            k= static_cast<unsigned char>(stre.get());//节拍器时钟(未知)
            l= static_cast<unsigned char>(stre.get());//一个四分音符包含几个三十二分音符
            h += '0';
            j = static_cast<int>(pow(2, j));
            k += '0';
            l += '0';
            tempo += h;
            tempo += '/';
            tempo += to_string(j);
            if (creatermode)cout << tempo << endl;
            break;
        case 0x59:
            if (creatermode)cout << "调号" << endl;
            unsigned char ud, bs;
            ud= static_cast<unsigned char>(stre.get());//获取升降号数
            bs= static_cast<unsigned char>(stre.get());//获取大小调
            break;
        }
        if (creatermode)cout << "__________________________" << endl;
    }


    Yingui(string data_) {
        const unsigned char FF = '\xff';
        const unsigned char F0 = '\xf0';
        data = data_;
        stre = stringstream(data);//将字串输入流
        stre.ignore(4);//跳过开头标识长度的内容
        unsigned char c;//暂存字元
        event_::event_type temp_previous = event_::event_type::null;//暂存上一个读取事件的类型
        while (have) {
            //cout << "__________________________" << endl;
            c = static_cast<unsigned char>(stre.get());
            //cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF) << " ";
            if (temp_previous == event_::stop) {//如果上一个是间隔事件，此事件可能是间隔事件以外的事件
                //cout << "test ";
                switch (c >> 4)
                {
                case 0x8: {//松开事件
                    //if(creatermode)cout << "松开";
                    play_up* u=new play_up(last, stre.get(),this);//创建
                    last = u;//记录指令链最后一个
                    stre.ignore(1);//忽略力度符号
                    temp_previous = event_::_8x;
                    break;
                }
                case 0x9: {//按下事件
                    //cout << "按下";
                    play_down* d=new play_down(last, stre.get(),this);//创建
                    last = d;//记录指令链最后一个
                    stre.ignore(1);//忽略力度符号
                    temp_previous = event_::_9x;
                    break;
                }
                case 0xc://设定乐器
                    //cout << "设定乐器"<<endl;
                    instruct = stre.get();
                    temp_previous = event_::_Cx;
                    break;
                case 0xf://f开头事件
                    if (creatermode)cout << "\n__________________________" << endl;
                    if (creatermode)cout << "f开头事件 ";
                    if (c == FF) {
                        if (creatermode)cout << "元事件 ";
                        unsigned char ty, len;
                        ty= static_cast<unsigned char>(stre.get());//获取事件类型
                        len= static_cast<unsigned char>(stre.get());//获取参数长度
                        other_handle(ty, len);
                        temp_previous = event_::_FF;
                    }//元事件
                    else {
                        if (creatermode)cout << "系统码";
                        stre.ignore(2);
                        temp_previous = event_::_F0;
                    }//系统码
                    break;
                case 0xa:
                    if (creatermode)cout << "无法呈现a";
                    stre.ignore(2);
                    temp_previous = event_::_Ax;
                    break;//原琴无法呈现这个
                case 0xb:
                    if (creatermode)cout << "无法呈现b";
                    stre.ignore(2);
                    temp_previous = event_::_Bx;
                    break;//原琴无法呈现这个
                case 0xd:
                    if (creatermode)cout << "无法呈现d";
                    stre.ignore(2);
                    temp_previous = event_::_Dx;
                    break;//原琴无法呈现这个
                case 0xe:
                    if (creatermode)cout << "无法呈现e";
                    stre.ignore(2);
                    temp_previous = event_::_Ex;
                    break;//原琴无法呈现这个
                default: {//仍是间隔事件
                    //cout << "间隔";
                    play_stop* s =new play_stop(last, stop_handle(c),this);
                    last = s;
                    temp_previous = event_::stop;
                    break;
                }
                }
            }
            else {//如果上一个事件不是间隔事件，那此事件必为间隔事件
                //cout << "间隔";
                play_stop* s =new play_stop(last, stop_handle(c),this);
                last = s;
                temp_previous = event_::stop;
            }
        }
    }

    void display() {
        cout << "音轨名: " << name << endl;
        cout << "备注: " << text << endl;
        cout << "版权信息: " << power << endl;
        cout << "乐器名称: " << instruct_name << endl;
        cout << "乐器代码: " << instruct << endl;
        cout << "总时间(ms): " << alltime << endl;
        cout << "__________________________" << endl;
    }
};

int main() {
    bool yunxing = true;
    bool fileagain = true;
    bool yinguiagain = true;
    while (yunxing) {
        if (fileagain)choice_file();
        if (yinguiagain)choice_yingui();
        startplay();
        fileagain = false;
        yinguiagain = false;
        int caozuo = -999;
        cout << "__________________________\n接下来的操作\n1—重新选择文件\n2—重新选择音轨\n3—重播\n4—关闭程序\n输入代号: ";

        do {
            if (caozuo != -999)cout << "输入内容非法，请重新输入: ";
            cin.clear();
            cin.ignore(pow(2, 31), '\n');
            cin >> caozuo;
        } while (caozuo > 4 || caozuo < 1);

        switch (caozuo) {
        case 1:
            fileagain = true; 
            yinguiagain = true;
            allclear();
            break;
        case 2:
            yinguiagain = true; 
            yinguiclear();
            break;
        case 3:
            threadclear();
            break;
        case 4:
            yunxing = false;
        }
    }
}

void choice_file() {
    allclear();
    txttomid tt;
    string s;
    int kk=-999;
    cout << "__________________________\n这里是原琴脚本\nBiliBili搜索三次元的混蛋,你将什么都搜不到,因为我还没上传\nGithub搜索sanciyuandehundan,你也搜不到代码,因为我没公开我的库\n(<ゝω0)☆哎嘿\n";
    //cout << "__________________________\n要传入哪种文件\n1—键盘谱\n2—midi档案\n";
    do {
        cin.clear();
        cout << "__________________________\n要传入哪种文件\n1—键盘谱\n2—midi档案\n输入代码: ";
        cin >> kk;
    } while (kk != 1 && kk != 2);
    if (kk == 1) {
        tt.transform();
        s = _getcwd(NULL,0);
        s += '\\';
        s += tt.file_name;
        s += ".mid";
    }
    else {
        cout << "请输入适配原琴的midi档完整路径: ";
        cin >> s;
    }
    read_mid(s);
    //read("C:\\Users\\a0905\\Downloads\\spiral.mid");
//read("C:\\Users\\a0905\\Downloads\\「spiral」- 无职转生S2 OP-V1.mid");
//read("C:\\Users\\a0905\\Downloads\\mid (6).mid");
    parse();
}
//选择文件

void choice_yingui() {
    //play_th(yingui[2]);
    for (int i = 1; i < yingui_num; i++) {
        cout << "__________________________" << endl;
        cout << "音轨代号: " << i << endl;
        yingui[i]->display();
    }
    //展示音轨信息，不包括全局音轨

    do {//检测输入内容是否不合理
        if (choice_num != -999) {
            cout << "输入内容非法,请重新输入" << endl;
        }
        cin.clear();
        cin.ignore(pow(2, 31), '\n');
        cout << "\n选择几个: ";
        cin >> choice_num;
    } while (choice_num >= yingui_num || choice_num <= 0);
    //输入选择几个

    for (int i = 0; i < choice_num; i++) {
        int temp = -999;
        bool same = false;
        do {//检测输入内容是否不合理
            if ((temp != -999) && (same || (temp >= choice_num || temp <= 0))) {
                cout << "输入内容非法,请重新输入" << endl;
            }
            cin.clear();
            cin.ignore(pow(2, 31), '\n');
            cout << "请输入音轨代号，数字即可: ";
            cin >> temp;
            same = false;
            for (int k = 0; k < i; k++) {
                if (temp == choice[k])same = true;
            }//是否有重复选择
            if (!same)choice[i] = temp;
        } while (same || (temp >= yingui_num || temp <= 0));
    }
    //输入选择哪几个
    cout << "选择: ";
    for (int i = 0; i < choice_num; i++) {
        cout << choice[i] << " ";
        if (yingui[choice[i]]->alltime > maxtime)maxtime = yingui[choice[i]]->alltime;//记录最长的音轨时间
    }
    cout << endl;
    //确认选择内容
}
//选择音轨

void startplay() {
    creat_th(choice, choice_num);//创建执行绪
    for (unsigned int i = 0; i < threads.size(); i++) {
        threads[i].detach();
    }//启动执行绪

    int stoptime;
    cout << "几秒后开始: ";
    while (!(cin >> stoptime)) {
        cin.clear();
        cin.ignore(pow(2, 31), '\n');
        cout << "请输入整数\n";
        cout << "几秒后开始: ";
    }
    printf("将在%d秒后开始播放\n", stoptime);
    this_thread::sleep_for(chrono::seconds(stoptime));
    //设定几秒后开始

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_start = true;
        cv.notify_all(); // 通知所有等待的线程开始执行
    }

    cout << "时长（ms）: " << dec << maxtime << endl;
    this_thread::sleep_for(chrono::microseconds(maxtime + 20000000));
}
//开始演奏

void read_mid(string path) 
{
    ifstream file(path, std::ios::binary); // 打开 MIDI 文件
    
    file_name = path.substr(path.find_last_of("\\") + 1);//去除父文件夹路径
    file_name = file_name.substr(0, file_name.find_last_of("."));//去除文件扩展名

    char byte; // 用于存储每个字节的临时变量
    if (file.is_open()) { // 检查文件是否成功打开
        while (file.read(&byte, sizeof(char))) { // 逐字节读取文件内容
            all += byte;
        }
        file.close(); // 关闭文件
        printf("\n%s读取成功！\n__________________________\n",file_name.c_str());
    }
    else {
        printf("__________________________\n无法打开%s文件,请检查路径是否正确\n",file_name.c_str()); // 如果无法打开文件则输出错误信息
        main();
    }
    
}
//读取文件到运行内存

void parse() {
    head = trans_16s_10int(all.substr(4, 4));//记录接下来几个字元是文件头
    if(creatermode)cout << "文件头长度: " << head<<endl;
    if (head != 6) {
        printf("文件格式非法,请选择其他文件");
        allclear();
        choice_file();
    }
    yingui_class = trans_16s_10int(all.substr(8, 2));//读取音轨类型
    if (creatermode)cout << "音轨类型: " << yingui_class << endl;
    if (yingui_class == 2) {
        printf("文件格式非法,请选择其他文件");
        allclear();
        choice_file();
    }
    yingui_num = trans_16s_10int(all.substr(10,2));//读取音轨数量
    if (creatermode)cout << "音轨数量: " << yingui_num << endl;

    time_tick = trans_16s_10int(all.substr(12,2));//读取一个四分音符几tick
    if (creatermode)cout << "一个四分音符几tick: " << time_tick<<endl;

    yingui_string = split(all, yingui_head);//将各个音轨分离储存
    yingui_string.erase(yingui_string.begin());//删除不是音轨的部分
    if (creatermode)cout << "\n__________________________"<<endl;
    for (string& yg : yingui_string) {//字串较长，为避免无意义复制而使用传指针
        //put_16(yg);
        //cout << "\n__________________________" <<endl;
        Yingui* y=new Yingui(yg);
        yingui.push_back(y);   
    }
    time_handle();
    for (Yingui* yg : yingui) {
        yg->start.handle_again();
    }
}
//解析文件头，分割音轨，调用音轨类的处理方法

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

void time_handle() {
    tick_weimiao = static_cast<double>(note_weimiao) / time_tick;
}//计算一tick几微妙

void play_th(Yingui* yg) {
    //this_thread::sleep_for(chrono::seconds(5));
    //cout << yg->data<<endl;
    //yg->display();
    yg->start.play_();
}
//调用初始事件的play函数以启动播放，用于创建执行绪

void creat_th(int* list,int num) {
    //thread th;
    for (int i = 1; i <= num; i++) {
        threads.push_back(thread(play_th, yingui[i]));
    }
}
//初始化执行绪

void allclear() {
    head = 0;//文件头多长
    note_weimiao = 500000;//一个四分音符多长(微秒)
    file_name;//文件名
    all = "";//这个文件，使用时尽量以引用为主，减少不必要复制
    yingui_string.clear();//以音轨为单位存储
    yingui_class = 0;//该文件音轨类型，0000单音轨，0001同步多音轨，0002异步多音轨(不受理)
    yingui_num = 1;//该文件音轨数量,全局音轨计算在内
    time_tick = 480;//该文件一个四分音符几tick
    ud = 0;//调号升降号数
    bs = 0;//大小调
    tick_weimiao = 0;//一个tick几微妙
    tempo = "";//记录节拍
    bell = '\0';//节拍器时钟
    four_32 = '\0';//一个四分音符包含几个三十二音符
    yingui.clear();//音轨
    threads.clear();//播放执行绪
    thread_start = false;
    choice = new int[yingui_num];//选择哪几个
    choice_num = -999;//选择几个
    maxtime = 0;
}
//清理内存，为下次演奏

void yinguiclear() {
    //yingui.clear();//音轨
    threads.clear();//播放执行绪
    choice = new int[yingui_num];//选择哪几个
    choice_num = -999;//选择几个
    maxtime = 0;
    thread_start = false;
}
//清理音轨选择，为重选音轨

void threadclear() {
    threads.clear();
    thread_start = false;
}
//清理执行绪，为重播

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
    for (unsigned char c : s) {
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