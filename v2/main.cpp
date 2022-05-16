#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <time.h>

using namespace sf;
using namespace std;

const int TreeWight = 60, TreeHeight = 60, GenomSize = 75, EvolveVal = 10, WorldW = 500, WorldH = 60,
 StepsToReborn = 40, RebornDeathVal = 5, RebornWeakVal = 8, RebornStrongVal = 8;
const float StartEnergy = 50, SunEnergyVal = 5.5, EarthEnergyVal = 0.65f, LoseEnergyVal = 1.5, LoseForNewCellVal = 5, UpdateCDVal = 100;
int ScreenSize[2], Step = 0, Generation = 0;
float UpdateCD = 0, scale = 1.0, xcomp = -1500, ycomp = 0;
bool TimeMode = false;
RenderWindow window;
Image allworld;

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
	int Genom[GenomSize][4], CoordInWorld, Old = 0;
	bool Died = false;
	Tree(int yyy = TreeHeight / 2)
	{
		body.create(TreeWight, TreeHeight, Color(0, 0, 0, 0));
		body.createMaskFromColor(Color(0, 0, 0));
		body.setPixel(TreeWight / 2 + 1, yyy, Color(0, 255, 0, 255));
	}
	bool IsThereCell(int x, int y)
	{
		return body.getPixel(x, y) != Color(0, 0, 0, 0) && body.getPixel(x, y) != Color(255, 0, 0, 0);
	}
	bool IsThereGlobalCell(int x, int y)
	{
		return allworld.getPixel(CoordInWorld + x, y) != Color(0, 0, 0) 
			&& allworld.getPixel(CoordInWorld + x, y) != Color(255, 0, 0);
	}
	bool IsReadyToGrewUp(int x, int y)
	{
		return body.getPixel(x, y).r == 0;
	}
	bool IsItSeed(int x, int y)
	{
		return body.getPixel(x, y).r == 255;
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
			if (IsThereGlobalCell(x, yy) || IsThereCell(x, yy)) val -= 1;
			else val += 1;
			if (val > 3) val = 3;
			if (val < 0) val = 0;
		}
		return ((SunEnergyVal * val) / 3.0f) * (float(TreeHeight / 2 - y) / float(TreeHeight) + 1.0f);
	}
	float DownEnergy(int x, int y)
	{
		float neighbours = -1;
		for (int xx = x - 1; xx <= x + 1; xx++)
			for (int yy = y - 1; yy <= y + 1; yy++)
				if (xx >= 0 && xx < TreeWight)
					if (yy > TreeHeight / 2 && yy < TreeHeight)
							neighbours += IsThereCell(xx, yy);
		return EarthEnergyVal * (8 - neighbours) * (float(y - TreeHeight / 2) / float(TreeHeight) + 1.0f);
	}
	void GenGenom()
	{
		for (int i = 0; i < GenomSize; i++)
		{
			Genom[i][0] = rand() % (GenomSize * 3);
			Genom[i][1] = rand() % (GenomSize * 3);
			Genom[i][2] = rand() % (GenomSize * 3);
			Genom[i][3] = rand() % (GenomSize * 3);
		}
	}
	bool Update() 
	{
		if (!Died)
		{
			Image bodybuf = body;
			for (int x = 0; x < TreeWight; x++)
				for (int y = 0; y < TreeHeight; y++)
				{
					if (IsThereCell(x, y))
					{
						if (CoordInWorld + x > WorldW) Energy = -1000;

						if (y <= TreeHeight / 2) Energy += UpEnergy(x, y);
						else Energy += DownEnergy(x, y);

						Energy -= LoseEnergyVal;
						if (IsReadyToGrewUp(x, y))
						{
							int GenNum = GetCellGenom(x, y);
							if (y > 0 && !IsThereGlobalCell(x, y - 1))
							{
								if (Genom[GenNum][0] < GenomSize)
								{
									Energy -= LoseForNewCellVal;
									bodybuf.setPixel(x, y - 1, Color(0, 255 - Genom[GenNum][0], 0, 255));
								}
								if (Genom[GenNum][0] >= GenomSize && Genom[GenNum][0] < GenomSize * 1.1 && y < TreeHeight / 2)
								{
									Energy -= LoseForNewCellVal * 5;
									bodybuf.setPixel(x, y - 1, Color(255, 0, 0, 255));
								}
							}
							if (y < TreeHeight - 1 && !IsThereGlobalCell(x, y + 1))
							{
								if (Genom[GenNum][1] < GenomSize)
								{
									Energy -= LoseForNewCellVal;
									bodybuf.setPixel(x, y + 1, Color(0, 255 - Genom[GenNum][1], 0, 255));
								}
								if (Genom[GenNum][1] >= GenomSize && Genom[GenNum][1] < GenomSize * 1.1 && y < TreeHeight / 2)
								{
									Energy -= LoseForNewCellVal * 5;
									bodybuf.setPixel(x, y + 1, Color(255, 0, 0, 255));
								}
							}
							if (x > 0 && x + CoordInWorld > 0 && !IsThereGlobalCell(x - 1, y))
							{
								if (Genom[GenNum][2] < GenomSize)
								{
									Energy -= LoseForNewCellVal;
									bodybuf.setPixel(x - 1, y, Color(0, 255 - Genom[GenNum][2], 0));
								}
								if (Genom[GenNum][2] >= GenomSize && Genom[GenNum][2] < GenomSize * 1.1 && y < TreeHeight / 2)
								{
									Energy -= LoseForNewCellVal * 5;
									bodybuf.setPixel(x - 1, y, Color(255, 0, 0, 255));
								}
							}
							if (x < TreeWight - 1 && x + CoordInWorld < WorldW - 1 && !IsThereGlobalCell(x + 1, y))
							{
								if (Genom[GenNum][3] < GenomSize)
								{
									Energy -= LoseForNewCellVal;
									bodybuf.setPixel(x + 1, y, Color(0, 255 - Genom[GenNum][3], 0));
								}
								if (Genom[GenNum][3] >= GenomSize && Genom[GenNum][3] < GenomSize * 1.1 && y < TreeHeight / 2)
								{
									Energy -= LoseForNewCellVal * 5;
									bodybuf.setPixel(x + 1, y, Color(255, 0, 0, 255));
								}
							}
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
			return false;
		}
		else return true;
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

struct Seed
{
	int Genom[GenomSize][4], CoordsInWorld[2];
	float Energy;
	Seed(int x, int y, float energy)
	{
		CoordsInWorld[0] = x; CoordsInWorld[1] = y; Energy = energy;
	}
};

vector<Tree> Trees;

vector<Seed> Seeds;

int main()
{
	allworld.create(WorldW, WorldH, Color(0, 0, 0, 0));
	srand(time(NULL));
	int startq = 30;
	for (int i = 0; i < 30; i++)
	{
		Trees.push_back(Tree());
		Trees[Trees.size() - 1].GenGenom();
		Trees[Trees.size() - 1].CoordInWorld = ((WorldW - 20) / startq) * i + ((WorldW - 20) / startq) / 2;
	}
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
	BackGroundSpr.setPosition(0, -float(ScreenSize[1]) / 2);
	window.create(VideoMode(ScreenSize[0], ScreenSize[1]), "Programm", Style::Fullscreen, ContextSettings(0, 0, 8));
	Clock clk;
	float presscd = 0;
	while (window.isOpen()) {
		cout << Trees.size() << endl;
		float TimesGone = float(clk.getElapsedTime().asMicroseconds()) * 0.001f; clk.restart();
		UpdateCD += TimesGone * (1.0 + 4.0 * TimeMode); 
		if (presscd < 300) presscd += TimesGone;
		while (UpdateCD >= UpdateCDVal)
		{
			UpdateCD -= UpdateCDVal; Step++;
			RenderTexture allworldrt;
			allworldrt.create(WorldW, WorldH);
			allworldrt.clear();
			for (int i = 0; i < Trees.size(); i++)
			{
				Texture bodyt; bodyt.loadFromImage(Trees[i].body);
				Sprite bodys; bodys.setTexture(bodyt);
				bodys.setPosition(Trees[i].CoordInWorld, 0);
				allworldrt.draw(bodys);
			}
			for (int i = 0; i < Seeds.size(); i++)
			{
				Image seeeeed; seeeeed.create(1, 1, Color(255, 0, 0));
				Texture seeeeedt; seeeeedt.loadFromImage(seeeeed);
				Sprite seeeeeds; seeeeeds.setTexture(seeeeedt);
				seeeeeds.setPosition(Seeds[i].CoordsInWorld[0], Seeds[i].CoordsInWorld[1]);
				allworldrt.draw(seeeeeds);
			}
			allworldrt.display();
			Texture allworldt = allworldrt.getTexture();
			allworld.copy(allworldt.copyToImage(), 0, 0, IntRect(0, 0, WorldW, WorldH), true);
			for (int i = 0; i < Seeds.size(); i++)
			{
				if (Seeds[i].CoordsInWorld[1] >= TreeHeight / 2)
				{
					Trees.push_back(Tree(Seeds[i].CoordsInWorld[1]));
					Trees[Trees.size() - 1].CoordInWorld = Seeds[i].CoordsInWorld[0] - TreeWight / 2;
					Trees[Trees.size() - 1].Energy = Seeds[i].Energy;
					for (int u = 0; u < GenomSize; u++)
					{
						Trees[Trees.size() - 1].Genom[u][0] = Seeds[i].Genom[u][0];
						Trees[Trees.size() - 1].Genom[u][1] = Seeds[i].Genom[u][1];
						Trees[Trees.size() - 1].Genom[u][2] = Seeds[i].Genom[u][2];
						Trees[Trees.size() - 1].Genom[u][3] = Seeds[i].Genom[u][3];
					}
					Trees[Trees.size() - 1].Evolve();
					Seeds.erase(Seeds.begin() + i); i -= 1;
				}
				else
				{
					if (allworld.getPixel(Seeds[i].CoordsInWorld[0], Seeds[i].CoordsInWorld[1] + 1) == Color(0, 0, 0))
					{
						Seeds[i].CoordsInWorld[1]++;
					}
					else
					{
						Seeds.erase(Seeds.begin() + i); i -= 1;
					}
				}
			}
			for (int i = 0; i < Trees.size(); i++)
			{
				if (Trees[i].CoordInWorld > WorldW + TreeWight / 2)
				{
					Trees.erase(Trees.begin() + i); i -= 1; break;
				}
				else
				{
					if (!Trees[i].Update())
					{
						Trees.erase(Trees.begin() + i); i -= 1; break;
					}
					else
					{
						Trees[i].Old++;
						if (Trees[i].Old >= StepsToReborn)
						{
							int qofseeds = 0;
							for (int x = 0; x < TreeWight; x++) for (int y = 0; y < TreeHeight; y++)
								qofseeds += Trees[i].IsItSeed(x, y);
							if (qofseeds > 0)
							{
								float Energyyy = Trees[i].Energy / float(qofseeds);
								for (int x = 0; x < TreeWight; x++) for (int y = 0; y < TreeHeight; y++)
									if (Trees[i].IsItSeed(x, y))
									{
										Seeds.push_back(Seed(Trees[i].CoordInWorld + x, y, Energyyy));
										for (int u = 0; u < GenomSize; u++)
										{
											Seeds[Seeds.size() - 1].Genom[u][0] = Trees[i].Genom[u][0];
											Seeds[Seeds.size() - 1].Genom[u][1] = Trees[i].Genom[u][1];
											Seeds[Seeds.size() - 1].Genom[u][2] = Trees[i].Genom[u][2];
											Seeds[Seeds.size() - 1].Genom[u][3] = Trees[i].Genom[u][3];
										}
									}
							}
							Trees.erase(Trees.begin() + i); i -= 1;
						}
					}
				}
			}
			allworld.createMaskFromColor(Color(0, 0, 0));
			clk.restart();
		}
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) window.close();
			if (event.type == sf::Event::MouseWheelScrolled)
			{
				float pastscale = scale;
				scale += event.mouseWheelScroll.delta * 0.01;
				//if ((float(float(ScreenSize[1]) / float(TreeHeight))) * scale * WorldW < ScreenSize[0]) scale = pastscale;
				if (scale < 0.1) scale = 0.1;
				xcomp = (xcomp / pastscale) * scale;
			}
		}
		Vector2i pixelPos = Mouse::getPosition(window);
		Vector2f pos = window.mapPixelToCoords(pixelPos);
		if (window.hasFocus())
		{
			if (Keyboard::isKeyPressed(Keyboard::Escape)) window.close();
			if (Keyboard::isKeyPressed(Keyboard::T) && presscd >= 300)
			{
				TimeMode = !TimeMode; presscd = 0;
			}
			xcomp -= (pos.x - (ScreenSize[0] / 2)) * TimesGone * 0.001;
			ycomp = pos.y - (ScreenSize[1] / 2);
			//if (xcomp > 0) xcomp = 0;
			//if (xcomp < -(float(float(ScreenSize[1]) / float(TreeHeight))) * scale * WorldW + ScreenSize[0])
			//	xcomp = -(float(float(ScreenSize[1]) / float(TreeHeight))) * scale * WorldW + ScreenSize[0];
		}
		window.clear();
		window.draw(BackGroundSpr);
		Texture allworldttt;
		allworldttt.loadFromImage(allworld);
		Sprite allworldsss;
		allworldsss.setTexture(allworldttt);
		allworldsss.setScale((float(float(ScreenSize[1]) / float(TreeHeight))) * scale, (float(float(ScreenSize[1]) / float(TreeHeight))));
		//allworldsss.setScale(5, 5);
		allworldsss.setPosition(xcomp, 0);
		window.draw(allworldsss);
		/*
		for (int i = 0; i < Trees.size(); i++)
		{
			Texture bodyt; bodyt.loadFromImage(Trees[i].body);
			Sprite bodys; bodys.setTexture(bodyt);
			bodys.setPosition(Trees[i].CoordInWorld + xcomp, 0);
			bodys.setScale(5, 5);
			window.draw(bodys);
		}*/
		text.setScale(1, 1);
		stringstream ss; ss << "Step: " << Step;
		text.setString(ss.str());
		text.setPosition(0, 0);
		window.draw(text);
		window.display();
	}
	return 0;
}