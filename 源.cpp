#include <vector>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <graphics.h>
#include "Gif.h"
using namespace std;
//1黑棋，-1白棋，0未下，mycolor指做决策一方的对手，-mycolor指做决策的一方

int board[9][9] = { 0 };
bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };
struct position {
	int x = -1;
	int y = -1;
};
vector <position> player, bot;
bool botwin = false, playerwin = false;//谁输谁赢

mouse_msg mouse = { 0 };
int redraw = 0;
key_msg keyMsg = { 0 };
PIMAGE start_bkg = newimage(1000, 800);
PIMAGE select_bkg = newimage(1000, 800);
PIMAGE mouse_pic = newimage(20, 20);
PIMAGE play_gamebkg = newimage(1000, 800);
PIMAGE winning_bkg = newimage(1000, 800);
PIMAGE play_file_bkg = newimage(1000, 800);
int mouseposx = 0, mouseposy = 0;

//决策部分
bool inBorder(int x, int y) //是否在棋盘内
{ return x >= 0 && y >= 0 && x < 9 && y < 9; }

bool dfs_air(int fx, int fy)//是否有气
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] == 0)
				flag = true;
			if (board[dx][dy] == board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(dx, dy))
					flag = true;
		}
	}
	return flag;
}

bool judgeAvailable(int fx, int fy, int color)//是否可以下color颜色的棋子
{
	if (board[fx][fy]) return false;
	board[fx][fy] = color;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(fx, fy))
	{
		board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(dx, dy))
				{
					board[fx][fy] = 0;
					return false;
				}
		}
	}
	board[fx][fy] = 0;
	return true;
}

//决策算法
struct node {
	int key = 0;//相当于用整数表示棋子的坐标xy
	double values = 0;//价值
};

node play(int mycolor);
unordered_map<int, double> scheme;//存储棋盘信息
double analyze(int x, int y, int round, int mycolor);
void update(int key, double& revalues, int mycolor);
int select();
void create(int mycolor);

void create(int mycolor)//创造对手策略集合
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (board[i][j] == 0)
				if (judgeAvailable(i, j, -mycolor))
				{
					if (i == 4 && j == 4 && mycolor == -1 && bot.empty())continue;
					//防止bot先手下正中间
					int key = i * 9 + j;
					scheme.emplace(key, 0);//如果可落子就放入容器
				}
}

double analyze(int x, int y, int round, int mycolor)//分析策略好坏
{
	int color = (round == 2 ? -mycolor : mycolor);//轮次
	double value = 0; int sum1 = 0, sum2 = 0;
	//价值判断(眼位)
	for (int i = 0; i < 4; i++)
	{
		int fx = x + cx[i]; int fy = y + cy[i];
		if (inBorder(fx, fy))//如果在棋盘内
		{
			sum1++;
			if (board[fx][fy] == board[x][y])//如果同色
				sum2++;
			if (board[fx][fy] == 0)//如果是空位置
			{
				board[fx][fy] = color;
				if (!dfs_air(fx, fy))//下了之后下的位置还有气
					value += (color / mycolor);//面对进攻时的价值
				else
				{
					board[fx][fy] = color;
					if (!dfs_air(x, y))//进攻价值
					{//下了之后中心位置没气
						if (round == 1)
						{
							board[x][y] = 0;
							value += 4 * (1 - analyze(fx, fy, 2, mycolor));//破坏对方收益
							board[x][y] = -mycolor;
						}
						else
							value += 4 * (color / mycolor);
					}
					else//下了之后中心位置还有气
						if (round == 1)
							value += analyze(fx, fy, 2, mycolor);//对方反制收益
				}
			}
		}
	}
	if (round == 1)
	{
		if (sum1 == sum2)
			return -99;
		value -= 3 * sum2;
		if (sum1 == 2)
			value -= 2;
	}
	value = value / round;
	return value;
}

void update(int key, double& revalues, int mycolor)//更新选择一种策略的价值
{
	int x = key / 9, y = key % 9;
	int memory[9][9] = { 0 };
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			memory[i][j] = board[i][j];
	board[x][y] = -mycolor;//己方落子
	revalues = analyze(x, y, 1, mycolor);
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = memory[i][j];
}

int select()//选取对手应对策略
{
	int maxkey;
	if (scheme.empty())//如果容器是空的
	{
		playerwin = true;
		return 88;
	}
	for (auto it = scheme.begin(); it != scheme.end(); it++)
	{
		double Values = it->second;
		if (Values == 0)
		{
			maxkey = it->first;
			return maxkey;
		}
	}//找还没评价过的策略
	return 99;
}

node play(int mycolor)//落子结果
{
	double maxvalue = -100;
	node result;//结果
	int number = 81;//设置测试次数
	node position[81];
	int member = 0;
	while (number--)
	{
		bool judge = 1;//防止多下一步
		int now = select();
		if (now == 88)//容器里面已经没有决策方案了
			return { 88,-1 };
		if (now == 99)//对手已经输了
			break;
		for (int i = 0; i < member; i++)
			if (position[i].key == now)
			{
				update(position[i].key, position[i].values, mycolor);
				scheme[position[i].key] = position[i].values;
				judge = 0;
			}
		if (judge)
		{
			position[member].key = now;
			update(position[member].key, position[member].values, mycolor);
			scheme[position[member].key] = position[member].values;
			member++;
		}
	}
	for (int i = 0; i < member; i++)
		if (maxvalue < position[i].values)
		{
			maxvalue = position[i].values;
			result = position[i];
		}
	return result;
}
//决策部分结束
//画图部分

void play_game(int mycolor);
void play_back(int mycolor);
bool jump = 1;
void create_file(int mycolor);
void play_file();
void winning();

void play_login()//入场动画
{
	PIMAGE temp3 = newimage();
	getimage(temp3, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\mouse1.jfif");//鼠标图片
	putimage(mouse_pic, 0, 0, 20, 20, temp3, 0, 0, getwidth(temp3), getheight(temp3));
	Gif gif(L"C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\loadinggiff.gif");
	gif.setPos(0, 0);
	gif.setSize(1000, 800);
	gif.info();
	//开始播放
	gif.play();
	for (; is_run(); delay_fps(60))
	{
		cleardevice();
		gif.draw();
		xyprintf(150, 0, "助教老师好，我是入学时长半年的计概生，求给分 (≧0≦)");
		xyprintf(250, 40, "按任意键播放暂停/继续；点击进入游戏");
		putimage(mouseposx - 10, mouseposy - 10, mouse_pic);
		while (kbmsg()) {
			keyMsg = getkey();
			if (keyMsg.msg == key_msg_down) {
				gif.toggle();
			}
		}
		while (mousemsg())
		{
			mouse = getmouse();
			mouseposx = mouse.x; mouseposy = mouse.y;
			if (mouse.is_up())
				redraw = 1;
		}
		while (redraw)
		{
			gif.clear();
			return;
		}
	}
}

void draw_start()//开始界面
{
	mouse = { 0 };
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\users\\wzshugo\\Pictures\\Saved Pictures\\guidao.png");
	putimage(start_bkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	redraw = 0;
	while (redraw == 0)
	{
		for (; is_run(); delay_jfps(60))//一秒循环60次
		{
			putimage(0, 0, start_bkg);
			rectangle(350, 550, 600, 625);
			rectangle(345, 545, 605, 630);
			rectangle(250, 425, 700, 500);
			rectangle(245, 420, 705, 505);
			rectangle(250, 300, 700, 375);
			rectangle(245, 295, 705, 380);
			if (mouseposx >= 250 && mouseposx <= 700 && mouseposy >= 300 && mouseposy <= 375)//画按钮
			{
				ege::setcolor(RED);
				rectangle(250, 300, 700, 375);
				rectangle(245, 295, 705, 380);
				ege::setcolor(WHITE);
			}
			else if (mouseposx >= 250 && mouseposx <= 700 && mouseposy >= 425 && mouseposy <= 500)
			{
				ege::setcolor(BLUE);
				rectangle(250, 425, 700, 500);
				rectangle(245, 420, 705, 505);
				ege::setcolor(WHITE);
			}
			else if (mouseposx >= 350 && mouseposx <= 600 && mouseposy >= 550 && mouseposy <= 625)
			{
				ege::setcolor(GREEN);
				rectangle(350, 550, 600, 625);
				rectangle(345, 545, 605, 630);
				ege::setcolor(WHITE);
			}
			putimage(mouseposx - 10, mouseposy - 10, mouse_pic);
			while (mousemsg())
			{
				mouse = getmouse();
				mouseposx = mouse.x; mouseposy = mouse.y;
				if (mouse.is_up() && mouse.x >= 250 && mouse.x <= 700 && mouse.y >= 300 && mouse.y <= 375)
					redraw = 1;
				if (mouse.is_up() && mouse.x >= 250 && mouse.x <= 700 && mouse.y >= 425 && mouse.y <= 500)
					redraw = 2;
				if (mouse.is_up() && mouse.x >= 350 && mouse.x <= 600 && mouse.y >= 550 && mouse.y <= 625)
					redraw = 3;
			}
			if (redraw)
				break;
		}
	}
}

void new_game()//新游戏界面
{
	mouse = { 0 };
	jump = 1;
	int mycolor = 0;
	botwin = false; playerwin = false;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = 0;//初始化落子情况
	player.clear();
	bot.clear();//初始化
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\select_bkg.png");
	putimage(select_bkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	setfillcolor(EGERGB(255, 205, 53));
	setbkcolor(WHITE);
	ege::setcolor(BLACK);
	setbkmode(TRANSPARENT);
	ege::setfont(40, 0, "楷体", 0, 300, 600, true, false, false);
	for (; is_run(); delay_jfps(60))
	{
		putimage(0, 0, select_bkg);
		rectangle(200, 170, 800, 230);
		while (mousemsg())
		{
			mouse = getmouse();
			mouseposx = mouse.x; mouseposy = mouse.y;
		}
		if (mouseposx >= 370 && mouseposx <= 630 && mouseposy >= 175 && mouseposy <= 225)
		{
			setfillcolor(RED);
			bar(370, 175, 630, 225);
			setfillcolor(EGERGB(255, 205, 53));
			if (mouse.is_up())
			{
				mycolor = 1;
				break;
			}
		}
		else
			bar(370, 175, 630, 225);
		if (mouseposx >= 630 && mouseposx <= 800 && mouseposy >= 175 && mouseposy <= 225)
		{
			setfillcolor(RED);
			bar(630, 175, 800, 225);
			setfillcolor(EGERGB(255, 205, 53));
			if (mouse.is_up())
			{
				mycolor = -1;
				break;
			}
		}
		else
			bar(630, 175, 800, 225);
		ege::outtextrect(205, 180, 600, 50, "我要执   黑棋（先手）   白棋   ");
	}
	play_game(mycolor);
}

void play_game(int mycolor)//游戏界面
{
	mouse = { 0 };
	if (mycolor == -1 && jump)
	{
		int fx = 0, fy = 0;
		create(mycolor);
		node what = play(mycolor);
		int result = what.key;
		fx = result / 9; fy = result % 9;
		position where = { fx,fy };
		board[fx][fy] = -mycolor;
		bot.push_back(where);
		scheme.clear();
		if (playerwin || botwin)
			winning();
	}
	jump = 1;
	bool next = false;
	int judge = 0;
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\play_gamebkg.png");
	putimage(play_gamebkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	ege_enable_aa(true);
	for (; is_run(); delay_jfps(30))
	{
		putimage(0, 0, play_gamebkg);
		bar(100, 75, 500, 475);
		for (int x = 140; x < 480; x += 40)
			line(x, 115, x, 435);
		for (int y = 115; y < 455; y += 40)
			line(140, y, 460, y);//画棋盘
		while (mousemsg())
		{
			mouse = getmouse();
			mouseposx = mouse.x; mouseposy = mouse.y;
		}
		for (int y = 50; y <= 550; y += 100)
			bar(600, y, 800, y + 60);//画按钮
		for (int y = 50; y <= 550; y += 100)
			if (mouseposx >= 600 && mouseposx <= 800 && mouseposy >= y && mouseposy <= y + 60)
			{
				setfillcolor(RED);
				bar(600, y, 800, y + 60);
				setfillcolor(EGERGB(255, 205, 53));
				if (mouse.is_up())
				{
					judge = (y - 150) / 100 + 2;
					break;
				}
			}//显示鼠标是否按在按钮上
		ege::outtextrect(630, 60, 200, 60, "读  档");
		ege::outtextrect(630, 160, 200, 60, "悔  棋");
		ege::outtextrect(630, 260, 200, 60, "保  存");
		ege::outtextrect(630, 360, 200, 60, "重  开");
		ege::outtextrect(630, 460, 200, 60, "认  输");
		ege::outtextrect(630, 560, 200, 60, "退  出");
		for (auto it = player.begin(); it != player.end(); it++)
		{
			if (mycolor == 1)
				setfillcolor(BLACK);
			else
				setfillcolor(WHITE);
			ege_fillellipse(130 + it->x * 40, 105 + 40 * it->y, 20, 20);
			setfillcolor(EGERGB(255, 205, 53));
		}//画棋子
		for (auto it = bot.begin(); it != bot.end(); it++)
		{
			if (mycolor == -1)
				setfillcolor(BLACK);
			else
				setfillcolor(WHITE);
			ege_fillellipse(130 + it->x * 40, 105 + 40 * it->y, 20, 20);
			setfillcolor(EGERGB(255, 205, 53));
		}//画棋子阴影
		int posx = mouseposx - 40 * ((mouseposx - 130) / 40) - 130;
		int posy = mouseposy - 40 * ((mouseposy - 105) / 40) - 105;
		int x = (mouseposx - 130) / 40; int y = (mouseposy - 105) / 40;
		if (posx >= 0 && posx <= 20 && posy >= 0 && posy <= 20 && board[x][y] == 0 && x >= 0 && x <= 8 && y >= 0 && y <= 8)//在棋盘内
		{
			if (mouse.is_up())
			{
				if (!judgeAvailable(x, y, mycolor))
				{
					botwin = true;
					next = true;
					break;
				}
				board[x][y] = mycolor;
				position newwhere = { x,y };
				player.push_back(newwhere);//储存
				next = true;
			}
			else
			{
				if (mycolor == 1)
					setfillcolor(0x255000000);
				else
					setfillcolor(0x255FCFCFC);
				ege_fillellipse(130 + x * 40, 105 + y * 40, 20, 20);
				setfillcolor(EGERGB(255, 205, 53));
			}//画棋子阴影
		}
		if (next || judge)
			break;
	}
	if (next)//决策过程
	{
		if (mycolor == 1)
		{
			int x = 0, y = 0;
			create(mycolor);
			node what = play(mycolor);
			int result = what.key;
			if (result != 88)
			{
				x = result / 9, y = result % 9;
				position where = { x,y };
				board[x][y] = -mycolor;
				bot.push_back(where);
				scheme.clear();//储存
			}
		}
		if (playerwin || botwin)
			winning();
		else
			play_game(mycolor);
	}
	else
		switch (judge)
		{
		case 1:play_file(); break;
		case 2:play_back(mycolor); break;
		case 3:create_file(mycolor); break;
		case 4:new_game(); break;
		case 5:botwin = true; winning();
		default:break;
		}
}

void play_back(int mycolor)//悔棋
{
	if (!player.size())
		for (; is_run(); delay_jfps(60))
		{
			xyprintf(80, 200, "无棋可悔，任意操作继续游戏");
			while (mousemsg() || kbmsg())
			{
				if (mycolor == -1)
					jump = 0;
				play_game(mycolor);
				return;
			}
		}
	position whereplayer = *(player.end() - 1);
	position wherebot = *(bot.end() - 1);
	player.pop_back();
	bot.pop_back();
	board[whereplayer.x][whereplayer.y] = 0;
	board[wherebot.x][wherebot.y] = 0;
	if (mycolor == -1)
		jump = 0;
	play_game(mycolor);
	return;
}

void create_file(int mycolor)//保存
{
	ofstream outtofile("lastgame.txt");
	if (!outtofile)
	{
		for (; is_run(); delay_jfps(60))
		{
			xyprintf(200, 200, "存档失败,任意操作返回");
			while (mousemsg() || kbmsg())
				play_game(mycolor);
		}
	}
	int size1 = bot.size(); int size2 = player.size();
	outtofile << mycolor << endl;
	outtofile << size1 << " " << size2 << endl;
	for (auto it = bot.begin(); it != bot.end(); it++)
		outtofile << it->x << " " << it->y << endl;
	for (auto it = player.begin(); it != player.end(); it++)
		outtofile << it->x << " " << it->y << endl;
	if (mycolor == 1)
		outtofile << 1;
	else
	{
		outtofile << 0;
		jump = 0;
	}
	outtofile.close();
	play_game(mycolor);
}

void play_file()//复盘界面（读档）
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = 0;
	bot.clear();
	player.clear();
	botwin = false; playerwin = false;
	jump = 1;
	ifstream infromfile("lastgame.txt");
	if (!infromfile)
	{
		for (; is_run(); delay_jfps(60))
		{
			xyprintf(200, 200, "存档读取失败\n任意操作开始新游戏");
			while (mousemsg() || kbmsg())
			{
				new_game();
				return;
			}
		}
	}
	int size1 = 0, size2 = 0;
	int mycolor = 0;
	infromfile >> mycolor;
	infromfile >> size1 >> size2;
	for (int i = 0; i < size1; i++)
	{
		int x = 0, y = 0;
		infromfile >> x >> y;
		board[x][y] = -mycolor;
		position where = { x,y };
		bot.push_back(where);
	}
	for (int i = 0; i < size2; i++)
	{
		int x = 0, y = 0;
		infromfile >> x >> y;
		board[x][y] = mycolor;
		position where = { x,y };
		player.push_back(where);
	}
	infromfile >> jump;
	ege::setfont(40, 0, "楷体", 0, 300, 600, true, false, false);
	play_game(mycolor);
}

void winning()//胜利界面
{
	bool judge = 0;
	mouse = { 0 };
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\winning_bkg.png");
	putimage(winning_bkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	for (; is_run(); delay_jfps(60))
	{
		putimage(0, 0, winning_bkg);
		while (mousemsg())
		{
			mouse = getmouse();
			mouseposx = mouse.x; mouseposy = mouse.y;
		}
		putimage(mouseposx - 10, mouseposy - 10, mouse_pic);
		ege::setcolor(YELLOW);
		ege::outtextrect(700, 50, 100, 40, "加油");
		ege::setcolor(RED);
		if (playerwin)
			ege::outtextrect(320, 200, 400, 80, "你战胜了AlphaNogo！");
		else
			ege::outtextrect(320, 200, 400, 80, "你被AlphaNogo击败！");
		if (mouseposx >= 200 && mouseposx <= 500 && mouseposy >= 350 && mouseposy <= 450)
		{
			setfillcolor(BROWN);
			bar(200, 350, 500, 450);
			setfillcolor(EGERGB(255, 205, 53));
			if (mouse.is_up())
			{
				judge++;
				break;
			}
		}
		else
			bar(200, 350, 500, 450);
		if (mouseposx >= 600 && mouseposx <= 900 && mouseposy >= 350 && mouseposy <= 450)
		{
			setfillcolor(BROWN);
			bar(600, 350, 900, 450);
			setfillcolor(EGERGB(255, 205, 53));
			if (mouse.is_up())
				break;
		}
		else
			bar(600, 350, 900, 450);
		ege::setcolor(GREEN);
		ege::outtextrect(240, 375, 490, 440, "再来一局！");
		ege::outtextrect(640, 375, 890, 440, "残忍离开? ");
	}
	if (judge)
		new_game();
	return;
}
//绘图结束

int main()
{
	board[3][7] = 1;
	board[5][3] = -1;
	//绘制棋盘
	initgraph(1000, 800, INIT_RENDERMANUAL);
	setcaption("AlphaNogo");
	setbkcolor(WHITE);
	ege::setcolor(BLACK);
	setbkmode(TRANSPARENT);
	ege::setfont(25, 0, "楷体");
	play_login();
	draw_start();
	switch (redraw)
	{
	case 1: new_game(); break;
	case 2: play_file(); break;
	default:ege::closegraph(); return 0;
	}
	ege::closegraph();
	return 0;
}
