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
//1���壬-1���壬0δ�£�mycolorָ������һ���Ķ��֣�-mycolorָ�����ߵ�һ��

int board[9][9] = { 0 };
bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };
struct position {
	int x = -1;
	int y = -1;
};
vector <position> player, bot;
bool botwin = false, playerwin = false;//˭��˭Ӯ

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

//���߲���
bool inBorder(int x, int y) //�Ƿ���������
{ return x >= 0 && y >= 0 && x < 9 && y < 9; }

bool dfs_air(int fx, int fy)//�Ƿ�����
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

bool judgeAvailable(int fx, int fy, int color)//�Ƿ������color��ɫ������
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

//�����㷨
struct node {
	int key = 0;//�൱����������ʾ���ӵ�����xy
	double values = 0;//��ֵ
};

node play(int mycolor);
unordered_map<int, double> scheme;//�洢������Ϣ
double analyze(int x, int y, int round, int mycolor);
void update(int key, double& revalues, int mycolor);
int select();
void create(int mycolor);

void create(int mycolor)//������ֲ��Լ���
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (board[i][j] == 0)
				if (judgeAvailable(i, j, -mycolor))
				{
					if (i == 4 && j == 4 && mycolor == -1 && bot.empty())continue;
					//��ֹbot���������м�
					int key = i * 9 + j;
					scheme.emplace(key, 0);//��������Ӿͷ�������
				}
}

double analyze(int x, int y, int round, int mycolor)//�������Ժû�
{
	int color = (round == 2 ? -mycolor : mycolor);//�ִ�
	double value = 0; int sum1 = 0, sum2 = 0;
	//��ֵ�ж�(��λ)
	for (int i = 0; i < 4; i++)
	{
		int fx = x + cx[i]; int fy = y + cy[i];
		if (inBorder(fx, fy))//�����������
		{
			sum1++;
			if (board[fx][fy] == board[x][y])//���ͬɫ
				sum2++;
			if (board[fx][fy] == 0)//����ǿ�λ��
			{
				board[fx][fy] = color;
				if (!dfs_air(fx, fy))//����֮���µ�λ�û�����
					value += (color / mycolor);//��Խ���ʱ�ļ�ֵ
				else
				{
					board[fx][fy] = color;
					if (!dfs_air(x, y))//������ֵ
					{//����֮������λ��û��
						if (round == 1)
						{
							board[x][y] = 0;
							value += 4 * (1 - analyze(fx, fy, 2, mycolor));//�ƻ��Է�����
							board[x][y] = -mycolor;
						}
						else
							value += 4 * (color / mycolor);
					}
					else//����֮������λ�û�����
						if (round == 1)
							value += analyze(fx, fy, 2, mycolor);//�Է���������
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

void update(int key, double& revalues, int mycolor)//����ѡ��һ�ֲ��Եļ�ֵ
{
	int x = key / 9, y = key % 9;
	int memory[9][9] = { 0 };
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			memory[i][j] = board[i][j];
	board[x][y] = -mycolor;//��������
	revalues = analyze(x, y, 1, mycolor);
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = memory[i][j];
}

int select()//ѡȡ����Ӧ�Բ���
{
	int maxkey;
	if (scheme.empty())//��������ǿյ�
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
	}//�һ�û���۹��Ĳ���
	return 99;
}

node play(int mycolor)//���ӽ��
{
	double maxvalue = -100;
	node result;//���
	int number = 81;//���ò��Դ���
	node position[81];
	int member = 0;
	while (number--)
	{
		bool judge = 1;//��ֹ����һ��
		int now = select();
		if (now == 88)//���������Ѿ�û�о��߷�����
			return { 88,-1 };
		if (now == 99)//�����Ѿ�����
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
//���߲��ֽ���
//��ͼ����

void play_game(int mycolor);
void play_back(int mycolor);
bool jump = 1;
void create_file(int mycolor);
void play_file();
void winning();

void play_login()//�볡����
{
	PIMAGE temp3 = newimage();
	getimage(temp3, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\mouse1.jfif");//���ͼƬ
	putimage(mouse_pic, 0, 0, 20, 20, temp3, 0, 0, getwidth(temp3), getheight(temp3));
	Gif gif(L"C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\loadinggiff.gif");
	gif.setPos(0, 0);
	gif.setSize(1000, 800);
	gif.info();
	//��ʼ����
	gif.play();
	for (; is_run(); delay_fps(60))
	{
		cleardevice();
		gif.draw();
		xyprintf(150, 0, "������ʦ�ã�������ѧʱ������ļƸ���������� (�R0�Q)");
		xyprintf(250, 40, "�������������ͣ/���������������Ϸ");
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

void draw_start()//��ʼ����
{
	mouse = { 0 };
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\users\\wzshugo\\Pictures\\Saved Pictures\\guidao.png");
	putimage(start_bkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	redraw = 0;
	while (redraw == 0)
	{
		for (; is_run(); delay_jfps(60))//һ��ѭ��60��
		{
			putimage(0, 0, start_bkg);
			rectangle(350, 550, 600, 625);
			rectangle(345, 545, 605, 630);
			rectangle(250, 425, 700, 500);
			rectangle(245, 420, 705, 505);
			rectangle(250, 300, 700, 375);
			rectangle(245, 295, 705, 380);
			if (mouseposx >= 250 && mouseposx <= 700 && mouseposy >= 300 && mouseposy <= 375)//����ť
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

void new_game()//����Ϸ����
{
	mouse = { 0 };
	jump = 1;
	int mycolor = 0;
	botwin = false; playerwin = false;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = 0;//��ʼ���������
	player.clear();
	bot.clear();//��ʼ��
	PIMAGE temp1 = newimage();
	getimage(temp1, "C:\\Users\\wzshugo\\Pictures\\Saved Pictures\\select_bkg.png");
	putimage(select_bkg, 0, 0, 1000, 800, temp1, 0, 0, getwidth(temp1), getheight(temp1));
	delimage(temp1);
	setfillcolor(EGERGB(255, 205, 53));
	setbkcolor(WHITE);
	ege::setcolor(BLACK);
	setbkmode(TRANSPARENT);
	ege::setfont(40, 0, "����", 0, 300, 600, true, false, false);
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
		ege::outtextrect(205, 180, 600, 50, "��Ҫִ   ���壨���֣�   ����   ");
	}
	play_game(mycolor);
}

void play_game(int mycolor)//��Ϸ����
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
			line(140, y, 460, y);//������
		while (mousemsg())
		{
			mouse = getmouse();
			mouseposx = mouse.x; mouseposy = mouse.y;
		}
		for (int y = 50; y <= 550; y += 100)
			bar(600, y, 800, y + 60);//����ť
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
			}//��ʾ����Ƿ��ڰ�ť��
		ege::outtextrect(630, 60, 200, 60, "��  ��");
		ege::outtextrect(630, 160, 200, 60, "��  ��");
		ege::outtextrect(630, 260, 200, 60, "��  ��");
		ege::outtextrect(630, 360, 200, 60, "��  ��");
		ege::outtextrect(630, 460, 200, 60, "��  ��");
		ege::outtextrect(630, 560, 200, 60, "��  ��");
		for (auto it = player.begin(); it != player.end(); it++)
		{
			if (mycolor == 1)
				setfillcolor(BLACK);
			else
				setfillcolor(WHITE);
			ege_fillellipse(130 + it->x * 40, 105 + 40 * it->y, 20, 20);
			setfillcolor(EGERGB(255, 205, 53));
		}//������
		for (auto it = bot.begin(); it != bot.end(); it++)
		{
			if (mycolor == -1)
				setfillcolor(BLACK);
			else
				setfillcolor(WHITE);
			ege_fillellipse(130 + it->x * 40, 105 + 40 * it->y, 20, 20);
			setfillcolor(EGERGB(255, 205, 53));
		}//��������Ӱ
		int posx = mouseposx - 40 * ((mouseposx - 130) / 40) - 130;
		int posy = mouseposy - 40 * ((mouseposy - 105) / 40) - 105;
		int x = (mouseposx - 130) / 40; int y = (mouseposy - 105) / 40;
		if (posx >= 0 && posx <= 20 && posy >= 0 && posy <= 20 && board[x][y] == 0 && x >= 0 && x <= 8 && y >= 0 && y <= 8)//��������
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
				player.push_back(newwhere);//����
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
			}//��������Ӱ
		}
		if (next || judge)
			break;
	}
	if (next)//���߹���
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
				scheme.clear();//����
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

void play_back(int mycolor)//����
{
	if (!player.size())
		for (; is_run(); delay_jfps(60))
		{
			xyprintf(80, 200, "����ɻڣ��������������Ϸ");
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

void create_file(int mycolor)//����
{
	ofstream outtofile("lastgame.txt");
	if (!outtofile)
	{
		for (; is_run(); delay_jfps(60))
		{
			xyprintf(200, 200, "�浵ʧ��,�����������");
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

void play_file()//���̽��棨������
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
			xyprintf(200, 200, "�浵��ȡʧ��\n���������ʼ����Ϸ");
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
	ege::setfont(40, 0, "����", 0, 300, 600, true, false, false);
	play_game(mycolor);
}

void winning()//ʤ������
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
		ege::outtextrect(700, 50, 100, 40, "����");
		ege::setcolor(RED);
		if (playerwin)
			ege::outtextrect(320, 200, 400, 80, "��սʤ��AlphaNogo��");
		else
			ege::outtextrect(320, 200, 400, 80, "�㱻AlphaNogo���ܣ�");
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
		ege::outtextrect(240, 375, 490, 440, "����һ�֣�");
		ege::outtextrect(640, 375, 890, 440, "�����뿪? ");
	}
	if (judge)
		new_game();
	return;
}
//��ͼ����

int main()
{
	board[3][7] = 1;
	board[5][3] = -1;
	//��������
	initgraph(1000, 800, INIT_RENDERMANUAL);
	setcaption("AlphaNogo");
	setbkcolor(WHITE);
	ege::setcolor(BLACK);
	setbkmode(TRANSPARENT);
	ege::setfont(25, 0, "����");
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
