#include "episodeplayergw.h"

EpisodePlayerGW::EpisodePlayerGW()
{

}

EpisodePlayerGW::EpisodePlayerGW(string mapTag)
{
    map.load(mapTag);
    initMap();
    agentShape->hide();
}

EpisodePlayerGW::EpisodePlayerGW(string mapTag, vector<vector<float>> sequence): sequence(sequence)
{
    map.load(mapTag);
    initMap();
    agentShape->setPos((sequence[0][1]+0.1)*SQUARE_SIZE,(sequence[0][0]+0.1)*SQUARE_SIZE);
    startShape->setPos(sequence[0][1]*SQUARE_SIZE,sequence[0][0]*SQUARE_SIZE);
    connect(&playClock,SIGNAL(timeout()),this,SLOT(update()));
}

void EpisodePlayerGW::initMap()
{
    gwView.setScene(&gwScene);
    gwScene.setSceneRect(0,0,map.getSize()*SQUARE_SIZE,map.getSize()*SQUARE_SIZE);
    gwView.setFixedSize(map.getSize()*SQUARE_SIZE,map.getSize()*SQUARE_SIZE);
    agentShape = new QGraphicsRectItem(0,0,4*SQUARE_SIZE/5,4*SQUARE_SIZE/5);
    startShape = new QGraphicsRectItem(0,0,SQUARE_SIZE,SQUARE_SIZE);
    arrivalShape = new QGraphicsRectItem(0,0,SQUARE_SIZE,SQUARE_SIZE);
    agentShape->setBrush(QBrush(Qt::magenta));
    agentShape->setZValue(1);
    gwScene.addItem(agentShape);
    startShape->setBrush(QBrush(Qt::yellow));
    gwScene.addItem(startShape);
    arrivalShape->setBrush(QBrush(Qt::green));
    gwScene.addItem(arrivalShape);
    for (int i=0;i<map.getSize();i++)
    {
        for (int j=0;j<map.getSize();j++)
        {
            switch(map.getMap()[i][j])
            {
                case 1:
                    obstacleShapes.append(new QGraphicsRectItem(0,0,SQUARE_SIZE,SQUARE_SIZE));
                    obstacleShapes.last()->setBrush(QBrush(Qt::red));
                    obstacleShapes.last()->setPos(SQUARE_SIZE*j,SQUARE_SIZE*i);
                    gwScene.addItem(obstacleShapes.last());
                    break;
                case 2:
                    arrivalShape->setPos(SQUARE_SIZE*j,SQUARE_SIZE*i);
                    break;
            }
        }
    }
}

void EpisodePlayerGW::showMap()
{
    gwView.show();
}

void EpisodePlayerGW::showScores(vector<vector<float>> scores)
{
    for (unsigned int i=0;i<scores.size();i++)
    {
        for (unsigned int j=0;j<scores.size();j++)
        {
            QGraphicsTextItem *score = new QGraphicsTextItem;
            score->setPos(SQUARE_SIZE*(j),SQUARE_SIZE*(i+0.25));
            string test = to_string(scores[i][j]);
            string test2;
            test2+=test[0];
            test2+=test[1];
            test2+=test[2];
            test2+=test[3];
            score->setPlainText(QString::fromStdString(test2));
            gwScene.addItem(score);
        }
    }
    showMap();
}

void EpisodePlayerGW::playEpisode()
{
    gwView.show();
    playClock.start(TIME_STEP);
    stepCount=0;
}

void EpisodePlayerGW::update()
{
    if (stepCount == sequence.size()-1)
    {
        playClock.stop();
    }
    else
    {
        stepCount++;
        agentShape->setPos((sequence[stepCount][1]+0.1)*SQUARE_SIZE,(sequence[stepCount][0]+0.1)*SQUARE_SIZE);
    }
}

MapGW EpisodePlayerGW::getMap()
{
    return map;
}

vector<vector<float>> EpisodePlayerGW::getSequence()
{
    return sequence;
}

void EpisodePlayerGW::setMap(MapGW map)
{
    this->map = map;
}

void EpisodePlayerGW::setSequence(vector<vector<float>> sequence)
{
    this->sequence=sequence;
}

EpisodePlayerGW::~EpisodePlayerGW()
{
    delete agentShape;
    delete startShape;
    delete arrivalShape;
    for (int i=0;i<obstacleShapes.length();i++)
    {
        delete obstacleShapes.at(i);
    }
}
