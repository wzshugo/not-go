#pragma once
#ifndef _EGEUTIL_GIF_H_
#define _EGEUTIL_GIF_H_

#include <time.h>
#include <gdiplus.h>
#include<graphics.h>
class Gif
{
private:
	int x, y;
	int width, height;
	int frameCount;					//֡��

	HDC hdc;						//�豸���
	Gdiplus::Graphics* graphics;	//ͼ�ζ���

	Gdiplus::Bitmap* gifImage;		//gifͼ��
	Gdiplus::PropertyItem* pItem;	//֡��ʱ����

	int curFrame;					//��ǰ֡
	clock_t pauseTime;				//��ͣʱ��

	clock_t	frameBaseTime;			//֡��׼ʱ��
	clock_t	curDelayTime;			//��ǰ֡���Ѳ���ʱ��
	clock_t	frameDelayTime;			//��ǰ֡������ʱʱ��

	bool playing;					//�Ƿ񲥷�
	bool visible;					//�Ƿ�ɼ�

public:
	Gif(const WCHAR* gifFileName = NULL, HDC hdc = getHDC(NULL));
	Gif(const Gif& gif);

	virtual ~Gif();

	Gif& operator=(const Gif& gif);

	//����ͼ��
	void load(const WCHAR* gifFileName);

	//���豸
	void bind(HDC hdc);
	void bindWindow();

	//��ռ���ͼ��
	void clear();

	//λ��
	void setPos(int x, int y);
	void setSize(int width, int height);

	int getX() const { return x; }
	int getY() const { return y; }

	//ͼ���С
	int getWidth() const { return width; }
	int getHeight() const { return height; }

	//ԭͼ��С
	int getOrginWidth() const;
	int getOrginHeight() const;

	//֡��Ϣ
	int getFrameCount() const { return frameCount; }
	int getCurFrame() const { return curFrame; }

	//��ʱʱ���ȡ������
	int getDelayTime(int frame) const;
	void setDelayTime(int frame, long time_ms);
	void setAllDelayTime(long time_ms);

	//����ʱ�䣬���㵱ǰ֡
	void updateTime();

	//���Ƶ�ǰ֡��ָ��֡
	void draw();
	void draw(int x, int y);
	void drawFrame(int frame);
	void drawFrame(int frame, int x, int y);

	//��ȡͼ��
	void getimage(PIMAGE pimg, int frame);

	//����״̬����
	void play();
	void pause();
	void toggle();

	bool isPlaying()const { return playing; }

	void setVisible(bool enable) { visible = enable; }
	bool isVisible() const { return visible; }

	bool isAnimation() const { return frameCount > 1; }

	//���ò���״̬
	void resetPlayState();

	void info() const;

private:
	void init();	//��ʼ��
	void read();	//��ȡͼ����Ϣ
	void copy(const Gif& gif);
};

#endif // !_EGEUTIL_GIF_H_


