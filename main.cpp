#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <time.h>
#include <vector>
#include <random>
#define _USE_MATH_DEFINES

// Preferences and prepaconstrations

const int windowWidth = 400, windowHeight = 400;
const int AGENTS_NUMBER = 5000;
const int SENSOR_LENGTH = 7; // MUST be greater than SENSOR_SIZE
const int SENSOR_SIZE = 2; //MUST be even
const double SENSOR_ANGLE = M_PI/4;
const int EDGE_L = 5;
const double TURN_ANGLE = M_PI/8;
const float TURN_RAND = 1.5;
const float EVAPO_RATE = 1;

const sf::Color COLOR_WHITE(255,255,255);
const sf::Color COLOR_GREEN(0,255,0,128);

static std::default_random_engine e;
static std::uniform_real_distribution<> dis(0, 1);

float randomValue(float min = -1, float max = 1, bool allowMiddle = false );

void setPixels(sf::Image& image, std::vector<std::vector<int>>& buffer, sf::Color color);

void evaporateImage(sf::Image& img);

sf::Color getPixelColor(sf::Image& image, int x, int y);

float getAvAreaValue(sf::Image& image, int x, int y, int a,  sf::Color favColor = COLOR_WHITE);


class Agent 
{
    public:
    float x, y, vx, vy;
    int bcounter = 0;

    Agent()
    {
        x = 0;
        y = 0;
        vx = 0;
        vy = 0;
    }
    Agent(float initial_x, float initial_y, float initial_vx, float initial_vy)
    {
        x = initial_x;
        y = initial_y;
        vx = initial_vx;
        vy = initial_vy;
    }

    void move(int windowWidth, int windowHeight)
    {
        if ((x > windowWidth - EDGE_L) || (x < EDGE_L))
        {
            vx = -vx;
        }
        if ((y > windowHeight - EDGE_L) || (y < EDGE_L))
        {
            vy = -vy;
        }
        
        float pvx = vx, pvy = vy;
        int rsd = randomValue()*TURN_RAND;

        vx = pvx*cos(rsd*TURN_ANGLE) - pvy*sin(rsd*TURN_ANGLE);
        vy = pvy*cos(rsd*TURN_ANGLE) + pvx*sin(rsd*TURN_ANGLE);

        x += vx;
        y += vy;
    }

    void search(sf::Image& image, sf::Color clr)
    {
        int lx, ly, srx, sry, slx, sly;
        float cvl, cvr;

        lx = SENSOR_LENGTH * vx / sqrt(pow(vx, 2) + pow(vy, 2));
        ly = SENSOR_LENGTH * vy / sqrt(pow(vx, 2) + pow(vy, 2));

        slx = int(x + lx*cos(SENSOR_ANGLE) - ly*sin(SENSOR_ANGLE));
        sly = int(y + ly*cos(SENSOR_ANGLE) + lx*sin(SENSOR_ANGLE));

        srx = int(x + lx*cos(-SENSOR_ANGLE) - ly*sin(-SENSOR_ANGLE));
        sry = int(y + ly*cos(-SENSOR_ANGLE) + lx*sin(-SENSOR_ANGLE));

        cvl = getAvAreaValue(image, slx, sly, SENSOR_SIZE, clr);
        cvr = getAvAreaValue(image, srx, sry, SENSOR_SIZE, clr);

        float pvx = vx, pvy = vy;

        if ((cvl == cvr) && (cvl*cvr !=0))
        {
            
            int rsd = randomValue()*TURN_RAND;

            vx = pvx*cos(rsd*TURN_ANGLE) - pvy*sin(rsd*TURN_ANGLE);
            vy = pvy*cos(rsd*TURN_ANGLE) + pvx*sin(rsd*TURN_ANGLE);
        }
        else
        { if (cvl < cvr )
            {
                vx = pvx*cos(-TURN_ANGLE) - pvy*sin(-TURN_ANGLE);
                vy = pvy*cos(-TURN_ANGLE) + pvx*sin(-TURN_ANGLE);
            }
        else
        {   if (cvl > cvr )
        {
                vx = (pvx*cos(TURN_ANGLE) - pvy*sin(TURN_ANGLE));
                vy = (pvy*cos(TURN_ANGLE) + pvx*sin(TURN_ANGLE));
            }   
        }
        }
        
    }

};

class Swarm
{
    public:
    int x, y;
    int agentCounter;
    const int size = windowHeight*0.25;
    std::vector<Agent> agents;
    sf::Color swarmColor;

    Swarm()
    {
        x = windowWidth/2;
        y = windowHeight/2; 
        agentCounter = 0;
    }

    Swarm(int ix,int iy, int agNum, sf::Color color)
    {
        x = ix;
        y = iy;
        agentCounter = agNum;
        swarmColor = color;

        for (int i =0; i < agentCounter; i++)
        {
            agents.push_back(Agent(x + sin(2*M_PI*i/agentCounter)*size/2,y + cos(2*M_PI*i/agentCounter)*size/2, randomValue(), randomValue()));
        }
    }

    void act(sf::Image& img)
    {
        std::vector<std::vector<int>> PixelBuffer;
        for (int i = 0; i < agentCounter; i++)
        {
            agents.at(i).search(img, swarmColor);
            agents.at(i).move(windowWidth, windowHeight);
            PixelBuffer.push_back({((int) agents.at(i).x), ((int) agents.at(i).y)});
            setPixels(img, PixelBuffer, swarmColor);
        }
    }
    
};

int main()
{
    int itercounter = 0;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "ITS ALIIIIVE!");
    std::vector<Agent> agents;
    
    sf::Image MImage;
    MImage.create(windowWidth, windowHeight);

    Swarm Swarm_1 = Swarm(windowWidth*3/4, windowHeight*3/4, AGENTS_NUMBER/2, sf::Color(255,255,255));
    Swarm Swarm_2 = Swarm(windowWidth/4, windowHeight/4, AGENTS_NUMBER/2, sf::Color(255,1,1));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        evaporateImage(MImage);
        
        Swarm_1.act(MImage);
        Swarm_2.act(MImage);
        
        sf::Texture MTexture;
        MTexture.loadFromImage(MImage); 
        sf::Sprite sprite(MTexture);
        window.draw(sprite);
        window.display();
        itercounter++;
    }

    return 0;
}

float randomValue(float min, float max, bool allowMiddle)
{
    float rcoef = dis(e);

    if (!allowMiddle)
    {
        while (rcoef == 0.5)
        {
            rcoef = dis(e);
        }
    }

    return rcoef*(max - min) + min;
}

void setPixels(sf::Image& image, std::vector<std::vector<int>>& buffer, sf::Color color){
    for (int i = 0; i < buffer.size(); i++)
    {
        if ((buffer.at(i).at(0) < image.getSize().x) && (buffer.at(i).at(0) > 0) && (buffer.at(i).at(1) < image.getSize().y) && (buffer.at(i).at(1) > 0))
        {
            image.setPixel(buffer.at(i).at(0), buffer.at(i).at(1), color);
        }
    }
    buffer.clear();
} 

void evaporateImage(sf::Image& img){
    for (int i = 0; i < img.getSize().x; i++)
    {
        for (int j = 0; j < img.getSize().y; j++)
        {
            sf::Color c = img.getPixel(i,j);
            c = sf::Color(c.r - (c.r >= EVAPO_RATE) * EVAPO_RATE, c.g - (c.g >= EVAPO_RATE) * EVAPO_RATE, c.b - (c.b >= EVAPO_RATE) * EVAPO_RATE);
            img.setPixel(i,j,c);
        }
    }
}

sf::Color getPixelColor(sf::Image& image, int x, int y){
    if ((x < image.getSize().x) && (x > 0) && (y < image.getSize().y) && y > 0)
    {
        return image.getPixel(x,y);
    }
    else
    {
        return sf::Color(0,0,0);
    }
}

float getAvAreaValue(sf::Image& image, int x, int y, int a, sf::Color favColor){
    int avg = 0;
    for (int i = -a/2; i <= a/2; i++){
        for (int j = -a/2; j <= a/2; j++){
            sf::Color colr = getPixelColor(image, x + i, y + j);
            if (colr.r + colr.g + colr.b != 0)
            {
                if ((colr.r / favColor.r == colr.g / favColor.g) && (colr.r / favColor.r == colr.b / favColor.b) && (colr.g / favColor.g == colr.b / favColor.b))
            {
                avg += 1;
            }
            else
            {
                avg -= 1;
            }
            }
        }
    }

    return avg/pow(a,2);
}