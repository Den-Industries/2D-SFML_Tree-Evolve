#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <time.h>

using namespace sf;
using namespace std;

const int TreeWight = 29, TreeHeight = 60, GenomSize = 75, EvolveVal = 40, 
CountOfTrees = 50, StepsToReborn = 250, RebornDeathVal = 5, RebornWeakVal = 5, RebornStrongVal = 10;
const float StartEnergy = 50, SunEnergyVal = 5.5, EarthEnergyVal = 0.7f, LoseEnergyVal = 3, UpdateCDVal = 100;
int ScreenSize[2], Step = 0;
float UpdateCD = 0, scale = 1.0, xcomp = 0, ycomp = 0;
bool TimeMode = false;
RenderWindow window;

void GetDesktopResolution()
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	ScreenSize[0] = GetSystemMetrics(SM_CXSCREEN);
	ScreenSize[1] = GetSystemMetrics(SM_CYSCREEN);
}

struct Tree
{
	Image body;
	float Energy = StartEnergy;
	int Genom[GenomSize][4];
	bool Died = false;
	Tree() 
	{
		body.create(TreeWight, TreeHeight, Color(0, 0, 0));
		body.createMaskFromColor(Color(0, 0, 0));
		body.setPixel(TreeWight / 2 + 1, TreeHeight  / 2, Color(0, 255, 0));
		for (int i = 0; i < 75; i++)
		{
			Genom[i][0] = rand() % (GenomSize * 3);
			Genom[i][1] = rand() % (GenomSize * 3);
			Genom[i][2] = rand() % (GenomSize * 3);
			Genom[i][3] = rand() % (GenomSize * 3);
		}
	}
	bool IsThereCell(int x, int y)
	{
		return body.getPixel(x, y) != Color(0, 0, 0, 0);
	}
	bool IsReadyToGrewUp(int x, int y)
	{
		return body.getPixel(x, y).r == 0;
	}
	int GetCellGenom(int x, int y)
	{
		return 255 - body.getPixel(x, y).g;
	}
	float UpEnergy(int x, int y)
	{
		float val = 3;
		for (int yy = 0; yy < y; yy++)
		{
			if (IsThereCell(x, yy)) val -= 1;
			else val += 1;
			if (val > 3) val = 3;
			if (val < 0) val = 0;
		}
		return ((SunEnergyVal * val) / 3.0f) * (float(TreeHeight / 2 - y) / float(TreeHeight) + 1.0f);
	}
	float DownEnergy(int x, int y)
	{
		int neighbours = -1;
		for (int xx = x - 1; xx <= x + 1; xx++)
			for (int yy = y - 1; yy <= y + 1; yy++)
				if (xx >= 0 && xx < TreeWight)
					if (yy > TreeHeight / 2 && yy < TreeHeight)
						neighbours += IsThereCell(xx, yy);
		return EarthEnergyVal * (8 - neighbours) * (float(y - TreeHeight / 2) / float(TreeHeight) + 1.0f);
	}
	void Update() 
	{
		if (!Died)
		{
			Image bodybuf = body;
			for (int x = 0; x < TreeWight; x++)
				for (int y = 0; y < TreeHeight; y++)
				{
					if (IsThereCell(x, y))
					{

						if (y <= TreeHeight / 2) Energy += UpEnergy(x, y);
						else Energy += DownEnergy(x, y);

						Energy -= LoseEnergyVal;
						if (IsReadyToGrewUp(x, y))
						{
							int GenNum = GetCellGenom(x, y);
							if (Genom[GenNum][0] < GenomSize && y > 0)
								bodybuf.setPixel(x, y - 1, Color(0, 255 - Genom[GenNum][0], 0));
							if (Genom[GenNum][1] < GenomSize && y < TreeHeight - 1)
								bodybuf.setPixel(x, y + 1, Color(0, 255 - Genom[GenNum][1], 0));
							if (Genom[GenNum][2] < GenomSize && x > 0)
								bodybuf.setPixel(x - 1, y, Color(0, 255 - Genom[GenNum][2], 0));
							if (Genom[GenNum][3] < GenomSize && x < TreeWight - 1)
								bodybuf.setPixel(x + 1, y, Color(0, 255 - Genom[GenNum][3], 0));
							Color buf = body.getPixel(x, y);
							buf.r = 1; bodybuf.setPixel(x, y, buf);
						}
					}
				}
			body = bodybuf;
		}
		if (Energy < 0)
		{
			Died = true;
			Energy = 0;
		}
	}
	void Evolve()
	{
		for (int i = 0; i < EvolveVal; i++)
			Genom[rand() % GenomSize][rand() % 4] = rand() % (GenomSize * 3);
	}
	void Born()
	{
		Died = false; Energy = StartEnergy;
		body.create(TreeWight, TreeHeight);
		body.createMaskFromColor(Color(0, 0, 0));
		body.setPixel(TreeWight / 2 + 1, TreeHeight / 2, Color(0, 255, 0));
	}
};

int main()
{
	srand(time(NULL));
	Tree Trees[CountOfTrees];
	GetDesktopResolution();
	Font font;
	font.loadFromFile("font.ttf");
	Text text("", font, 48);
	text.setFillColor(Color(50, 20, 20));
	text.setStyle(sf::Text::Bold);
	Image BackGroundIm; 
	BackGroundIm.create(1, 2, Color(200, 200, 255));
	BackGroundIm.setPixel(0, 1, Color(180, 120, 80));
	Texture BackGroundTxtr;
	BackGroundTxtr.loadFromImage(BackGroundIm);
	Sprite BackGroundSpr;
	BackGroundSpr.setTexture(BackGroundTxtr);
	BackGroundSpr.setScale(float(ScreenSize[0]), float(ScreenSize[1]));
	window.create(VideoMode(ScreenSize[0], ScreenSize[1]), "Programm", Style::Fullscreen, ContextSettings(0, 0, 8));
	vector <pair <float, int>> TreeList;
	Clock clk;
	while (window.isOpen()) {

		float TimesGone = float(clk.getElapsedTime().asMicroseconds()) * 0.001f; clk.restart();
		UpdateCD += TimesGone * (1 + 9 * TimeMode);
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) window.close();
			if (event.type == sf::Event::MouseWheelScrolled)
			{
				scale += event.mouseWheelScroll.delta * 0.01;
				if (scale < 0.1) scale = 0.1;
			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape)) window.close();
		if (Keyboard::isKeyPressed(Keyboard::T))
		{
			TimeMode = !TimeMode;
			Sleep(300);
		}
		window.clear();
		if ((float(float(ScreenSize[1]) / float(TreeHeight))) * scale * TreeHeight > ScreenSize[1]) 
			BackGroundSpr.setPosition(0, ycomp - float(ScreenSize[1]) / 2);
		else BackGroundSpr.setPosition(0, -float(ScreenSize[1]) / 2);
		window.draw(BackGroundSpr);
		float xcord = 0;
		for (int i = 0; i < CountOfTrees; i++)
			if (xcord + xcomp < ScreenSize[0])
			{
				Texture txtr; 
				txtr.loadFromImage(Trees[i].body);
				Sprite spr;
				spr.setTexture(txtr);
				spr.setScale((float(float(ScreenSize[1]) / float(TreeHeight))) * scale, (float(float(ScreenSize[1]) / float(TreeHeight))) * scale);
				if ((float(float(ScreenSize[1]) / float(TreeHeight))) * scale * TreeHeight > ScreenSize[1])
					spr.setPosition(xcord + xcomp, (ScreenSize[1] / 2) - spr.getGlobalBounds().height / 2 + ycomp);
				else
					spr.setPosition(xcord + xcomp, (ScreenSize[1] / 2) - spr.getGlobalBounds().height / 2);
				xcord += spr.getGlobalBounds().width;
				text.setScale(scale, scale);
				if (Trees[i].Died)
				{
					spr.setColor(Color(255, 64, 255));
					text.setString("DIED");
					text.setPosition(spr.getPosition().x, spr.getPosition().y + 100 * scale);
					window.draw(text);
				}
				stringstream ss; ss << "Energy: " << int(Trees[i].Energy);
				text.setString(ss.str());
				text.setPosition(spr.getPosition().x, spr.getPosition().y + 50 * scale);
				window.draw(text);
				window.draw(spr);
			}
		Vector2i pixelPos = Mouse::getPosition(window);
		Vector2f pos = window.mapPixelToCoords(pixelPos);
		xcomp -= (pos.x - (ScreenSize[0] / 2)) * TimesGone * 0.001;
		if (xcomp > 0) xcomp = 0;
		ycomp = pos.y - (ScreenSize[1] / 2);
		text.setScale(1, 1);
		stringstream ss; ss << "Step: " << Step;
		text.setString(ss.str());
		text.setPosition(0, 0);
		window.draw(text);
		window.display();
		if (UpdateCD >= UpdateCDVal)
		{
			UpdateCD -= UpdateCDVal; Step++;
			for (int i = 0; i < CountOfTrees; i++)
				Trees[i].Update();
		}
		if (Step == StepsToReborn)
		{
			TreeList.clear(); Step = 0;
			float AllEnergy = 0;
			for (int i = 0; i < CountOfTrees; i++)
			{
				AllEnergy += Trees[i].Energy;
				TreeList.push_back({ Trees[i].Energy, i });
			}
			//for (int u = 0; u < TreeList.size(); u++)
			//	cout << TreeList[u].first << " " << TreeList[u].second << endl;
			sort(TreeList.begin(), TreeList.end());
			reverse(TreeList.begin(), TreeList.end());
			//for (int u = 0; u < TreeList.size(); u++)
			//	cout << TreeList[u].first << " " << TreeList[u].second << endl;
			int HowMany = 0;
			for (int i = 0; i < CountOfTrees; i++)
				if (Trees[i].Died)
				{
					int Strong;
					for (int u = 0; u < TreeList.size(); u++)
						if (rand() % RebornStrongVal == 0)
						{
							Strong = TreeList[u].second;
							break;
						}
					for (int u = 0; u < 75; u++)
					{
						Trees[i].Genom[u][0] = Trees[Strong].Genom[u][0];
						Trees[i].Genom[u][1] = Trees[Strong].Genom[u][1];
						Trees[i].Genom[u][2] = Trees[Strong].Genom[u][2];
						Trees[i].Genom[u][3] = Trees[Strong].Genom[u][3];
					}
					Trees[i].Evolve(); HowMany++;
				}
			if (HowMany < RebornDeathVal)
			{
				for (int j = 0; j < RebornDeathVal - HowMany; j++)
				{
					reverse(TreeList.begin(), TreeList.end());
					int Weak;
					for (int u = 0; u < TreeList.size(); u++)
						if (rand() % RebornWeakVal == 0)
						{
							Weak = TreeList[u].second;
							break;
						}
					reverse(TreeList.begin(), TreeList.end());
					int Strong;
					for (int u = 0; u < TreeList.size(); u++)
						if (rand() % RebornStrongVal == 0)
						{
							Strong = TreeList[u].second;
							break;
						}
					for (int u = 0; u < 75; u++)
					{
						Trees[Weak].Genom[u][0] = Trees[Strong].Genom[u][0];
						Trees[Weak].Genom[u][1] = Trees[Strong].Genom[u][1];
						Trees[Weak].Genom[u][2] = Trees[Strong].Genom[u][2];
						Trees[Weak].Genom[u][3] = Trees[Strong].Genom[u][3];
					}
					Trees[Weak].Evolve();
				}
			}
			for (int i = 0; i < CountOfTrees; i++) Trees[i].Born();
		}
	}
	return 0;
}