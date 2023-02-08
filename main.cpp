#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>
#define _USE_MATH_DEFINES

// Preferences and prepaconstrations

const int windowWidth = 512, windowHeight = 512;
const bool fullScreen = false;
const bool renderTexture = false; //Save every 24th frame to drive, needs /render subdir created near program
const int AGENTS_NUMBER = 10000;
const int SPAWN_TYPE = 3; //RGB_TRI, BW, R_VS_W
const int AGENT_SPEED = 1;
const int SENSOR_LENGTH = 8; // MUST be greater than SENSOR_SIZE
const int SENSOR_SIZE = 2; //MUST be even
const double SENSOR_ANGLE = M_PI/4;
const int EDGE_L = 0; //not needed, but left just in case, adds a no-go zone on borders
const double TURN_ANGLE = M_PI/8;
float TURN_RAND = 1.2; //1.2
const float EVAPO_RATE = 0.997;//0.997
// ==== Ironically, shaders lag on higher resoultions, so use with caution
const bool USE_SHADERS = false; //i advise to switch TURN_RAND to 1.01 and EVAPO_RATE to 0.92
const float BLUR_RAD = 0.002;
const float CUTOUT_V = 0.005;


//======Classic colors===================
const sf::Color COLOR_BLUE(1,1,255);
const sf::Color COLOR_RED(255,1,1);
const sf::Color COLOR_GREEN(1,255,1);
const sf::Color COLOR_WHITE(255,255,255);
const sf::Color BG_COLOR(0,0,0);

//======Moss colors======================
// const sf::Color COLOR_BLUE(1,200,1);
// const sf::Color COLOR_RED(1,200,1);
// const sf::Color COLOR_GREEN(1,200,1);
// const sf::Color BG_COLOR(76,76,76);

static std::default_random_engine e;
static std::uniform_real_distribution<> dis(0, 1);


float randomValue(float min = -1, float max = 1, bool allowMiddle = false );

void setPixels(sf::Image& image, std::vector<std::vector<int>>& buffer, sf::Color color);

void evaporateImage(sf::Image& img);

float getAvAreaValue(sf::Image& image, int x, int y, int a,  sf::Color favColor = BG_COLOR);


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
        if (sqrt(pow(vx,2) + pow(vy,2)) <= 0.5)
        {
            vx = randomValue();
            vy = randomValue();
        }
        else
        {
            float pvx = vx, pvy = vy;
        int rsd = randomValue()*TURN_RAND;

        vx = pvx*cos(rsd*TURN_ANGLE) - pvy*sin(rsd*TURN_ANGLE);
        vy = pvy*cos(rsd*TURN_ANGLE) + pvx*sin(rsd*TURN_ANGLE);
        }

        x += vx;
        y += vy;
        if (x > windowWidth - 1 - EDGE_L)
        {
            x = EDGE_L;
        }
        if  (x < EDGE_L)
        {
            x = windowWidth - 1 - EDGE_L;
        }
        if (y > windowHeight - 1 - EDGE_L)
        {
            y = EDGE_L ;
        }
        if  (y < EDGE_L)
        {
            y = windowHeight - 1 - EDGE_L;
        }
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
            int rsd = randomValue();

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
    int size;
    std::vector<Agent> agents;
    sf::Color swarmColor;

    Swarm()
    {
        x = windowWidth/2;
        y = windowHeight/2; 
        agentCounter = 0;
    }

    Swarm(int ix,int iy, int agNum, sf::Color color, int sz = windowHeight*0.1)
    {
        x = ix;
        y = iy;
        agentCounter = agNum;
        swarmColor = color;
        size = sz;

        for (int i =0; i < agentCounter; i++)
        {
            agents.push_back(Agent(x + randomValue(0,1,true)*sin(2*M_PI*i/agentCounter)*size/2,y + randomValue(0,1,true)*cos(2*M_PI*i/agentCounter)*size/2, AGENT_SPEED*randomValue(), AGENT_SPEED*randomValue()));
        }

    }

    void act(sf::Image& img)
    {
        std::vector<std::vector<int>> PixelBuffer;
        std::for_each( agents.begin(), agents.end(), [&](Agent& agent)
        {
            agent.search(img,swarmColor);
            agent.move(windowWidth,windowHeight);
            img.setPixel(agent.x, agent.y, swarmColor);
        });
    }
    
};

int main()
{
    e.seed(time(0));
    int itercounter = 0;
    time_t start;
    double alltime = 0;
    sf::RenderWindow window;
    sf::Shader blurShader;

    std::vector<Swarm> swarms;
    int SwarmCtr;

    if (fullScreen)
    {
        window.create(sf::VideoMode(windowWidth, windowHeight), "ITS ALIIIIVE!",sf::Style::Fullscreen); //sf::Style::Fullscreen
    }
    else
    {
        window.create(sf::VideoMode(windowWidth, windowHeight), "ITS ALIIIIVE!"); //sf::Style::Fullscreen
    }


    std::vector<Agent> agents;

    sf::Image MImage;
    sf::Texture MTexture;
    sf::RenderTexture RTexture;
    MImage.create(windowWidth, windowHeight, BG_COLOR);
    blurShader.loadFromFile("blur.frag", sf::Shader::Fragment);
    blurShader.setUniform("texture",sf::Shader::CurrentTexture);
    blurShader.setUniform("blur_radius",BLUR_RAD);
    blurShader.setUniform("evaporate",EVAPO_RATE);
    blurShader.setUniform("cutout",CUTOUT_V);
    RTexture.create(windowWidth,windowHeight);
    switch (SPAWN_TYPE) //RGB_TRI, RGB_CIRCLES, BW
    {
        case 1:
            SwarmCtr = 3;
            swarms.push_back(Swarm(windowWidth/2, windowHeight/2, AGENTS_NUMBER/3, COLOR_GREEN, windowHeight/4));
            swarms.push_back(Swarm(windowWidth/2, windowHeight/2, AGENTS_NUMBER/3, COLOR_BLUE, windowHeight/3));
            swarms.push_back(Swarm(windowWidth/2, windowHeight/2, AGENTS_NUMBER/3, COLOR_RED, windowHeight/1.5 - 50));
            break;
        case 2:
            SwarmCtr = 1;
            swarms.push_back(Swarm(windowWidth/2, windowHeight/2, AGENTS_NUMBER, COLOR_WHITE, 1));
            break;
        case 3:
            SwarmCtr = 2;
            swarms.push_back(Swarm(windowWidth/4, windowHeight/4, AGENTS_NUMBER/2, COLOR_WHITE, 1));
            swarms.push_back(Swarm(windowWidth*3/4, windowHeight*3/4, AGENTS_NUMBER/2, COLOR_RED, 1));
            break;
    }



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                std:: cout << "Average IPS: " << alltime/itercounter << "\n";
                window.close();
            }
        }

        window.clear();
        if (USE_SHADERS)
        {
        MTexture.loadFromImage(MImage);
        sf::Sprite sprite0(MTexture);
        RTexture.clear(BG_COLOR);
        RTexture.draw(sprite0, &blurShader);
        RTexture.display();
        MTexture = RTexture.getTexture();
        MImage = MTexture.copyToImage();
        }
        else{
        evaporateImage(MImage);
        }
	
        start = std::clock();

        for (int i = 0; i<SwarmCtr; i++)
        {
            swarms[i].act(MImage);
        }


        MTexture.loadFromImage(MImage);
        sf::Sprite sprite(MTexture);
        window.draw(sprite);
        window.display();
        if (renderTexture)
        {
            if (itercounter%24 == 0)
            {
                MTexture.copyToImage().saveToFile("render/" + std::to_string(itercounter/24) + ".png");
            }
        }
        alltime += (double) CLOCKS_PER_SEC / (std::clock() - start);
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

void evaporateImage(sf::Image& img){
    for (int i = 1; i < img.getSize().x - 1; i++)
    {
        for (int j = 1; j < img.getSize().y - 1; j++)
        {
            sf::Color c = img.getPixel(i,j);
            if (c != BG_COLOR)
            {
                c = sf::Color(int(c.r*EVAPO_RATE + BG_COLOR.r*(1-EVAPO_RATE)),int(c.g*EVAPO_RATE + BG_COLOR.g*(1-EVAPO_RATE)),int(c.b*EVAPO_RATE + BG_COLOR.b*(1-EVAPO_RATE)));
                img.setPixel(i,j,c);
            }
        }
    }
}

float getAvAreaValue(sf::Image& image, int x, int y, int a, sf::Color favColor){
    int avg = 0;
    int f_x = 0;
    int f_y = 0;
    for (int i = -a/2; i <= a/2; i++){
        for (int j = -a/2; j <= a/2; j++){
            f_x = x+i;
            f_y = y+j;
            if (f_x > windowWidth-1)
            {
                f_x -= windowWidth;
            }
            else{if (f_x < 0)
            {
                f_x +=  windowWidth;
            }}
            if (f_y > windowHeight-1)
            {
                f_y -= windowHeight;
            }
            else{if (f_y < 0)
            {
                f_y +=  windowHeight;
            }}
            sf::Color colr = image.getPixel(f_x,f_y);
            if (colr != BG_COLOR)
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
    return avg;
}
