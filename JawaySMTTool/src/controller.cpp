#include "controller.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <QDebug>

using namespace std;
Controller::Controller()
{
    m_proto  = nullptr;
    //m_widget = nullptr;
}

void Controller::clearConfigMap()
{
    m_proto->config_map.clear();
}

void Controller::loadConfig() //config.yaml --> config_map
{
    ifstream f_in("./config.yaml");
    if(!f_in)
    {
        qDebug() << "read config.yaml error";
        return;
    }

    YAML::Node config = YAML::Load(f_in);
    if(!config.IsDefined()) //if(config.IsNull())
    {
        qDebug() << "yaml load error";
        goto LOAD_END;
    }

    clearConfigMap();

    for(YAML::const_iterator it=config.begin(); it!=config.end(); it++)
    {
        string node_name = it->first.as<string>();
        string enable;
        string type;
        string value;
        if(config[node_name]["enable"])
        {
            enable = config[node_name]["enable"].as<string>();
        }
        if(config[node_name]["type"])
        {
            type = config[node_name]["type"].as<string>();
            if(config[node_name]["value"])
            {
                value = config[node_name]["value"].as<string>();
            }
        }

        //cout << node_name << ":" << enable << "," << type << "," << value << endl;
        //------------------------------------------------------------------------
        item_t item = {enable, type, value};
        setConfigMap(m_proto, node_name, &item);
    }

    LOAD_END:
    f_in.close();
}

void Controller::setConfigMap(Protocol *proto, string node_name,item_t *p_item)
{
    proto->config_map[node_name] = *p_item;
    //qDebug() << "config_map:" << QString::fromStdString(node_name)<< "---" << QString::fromStdString(p_item->value);
}

void Controller::saveConfig() //config_map --> config.yaml
{
    ofstream f_out("./config.yaml");
    if(!f_out)
    {
        qDebug() << "open config.yaml error";
    }

    //config_map ---> config.yaml
    YAML::Node config;
    unordered_map<string, item_t>::iterator iter;
    for(iter=m_proto->config_map.begin(); iter!=m_proto->config_map.end(); iter++)
    {
        string node_name = iter->first;
        item_t item = iter->second;
        //config[node_name] = item;
        config[node_name]["enable"] = item.enable;
        config[node_name]["type"] = item.type;
        config[node_name]["value"] = item.value;
    }
    f_out << config;
    f_out.close();
}

void Controller::setHomeMap()
{
    //config_map --> home_map
    unordered_map<string, item_t> *p_item = &m_proto->config_map;
    unordered_map<string, item_t>::iterator iter;
    for(iter=p_item->begin(); iter!=p_item->end(); iter++)
    {
        string enable = iter->second.enable;
        //if(enable == "1")
        {
            string node_name = iter->first;
            string set_value = iter->second.value;
            string type      = iter->second.type;
            m_proto->home_map[node_name] = item_t{enable, type, set_value};
            qDebug() << "home_map:" << QString::fromStdString(node_name) << QString::fromStdString(set_value);
        }
    }
}
