#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <protocol.h>
//#include <widget.h>

class Controller :public QObject
{
    Q_OBJECT
public:
    Controller();

    Protocol *m_proto;
    //Widget   *m_widget;

    void loadConfig();
    void saveConfig();

    void setConfigMap(Protocol *proto,string node_name,item_t *p_item);//map and page
    void clearConfigMap();

    //void updateConfig();//config_map and config.yaml

    void setHomeMap(); //home_map <-- config_map
    void clearHome();

    void dispUpdate();//home update


};

#endif // CONTROLLER_H
