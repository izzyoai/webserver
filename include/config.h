#ifndef CONFIG_H
#define CONFIG_H

#include<fstream>
#include<string>
#include<iostream>
#include<map>
#include<vector>


class Config{

public:

    Config(std::string fileName):result(false)
    {
        std::ifstream ifs;
        ifs.open(fileName,std::ios::in);
        if(!ifs.is_open())
        {
            ifs.close();
            return;
        }

        _readConfigFile(ifs);

        ifs.close();

    }

    void ReLoadFile(std::string fileName)
    {
        std::ifstream ifs;
        ifs.open(fileName,std::ios::in);
        //清空map
        config.clear();
        result = false;

        if(!ifs.is_open())
        {
            ifs.close();
            return;
        }

        ifs.close();
    }
    
    bool Is_result()
    {
        return result;
    }

    void TransferConfig(std::map<std::string,std::string>& m)
    {
        for(auto iter = config.cbegin();iter != config.cend();iter++)
        {
            std::pair<std::string,std::string> kv(iter->first,iter->second);
            m.insert(kv);
        }
    }

private:
    void _readConfigFile(std::ifstream& ifs)
    {
        //注释和匹配
        char note = '#';
        char mate = '=';
        //中间缓冲
        bool is_mate;
        std::string buf;
        std::string key;
        std::string value;

        while(ifs.peek()!=EOF)
        {
            ifs>>buf;
            key = "";
            value = "";
            is_mate = false;
            for(auto iter = buf.begin();iter != buf.end();iter++)
            {
                if((*iter) == note)
                {
                    break;
                }
                if((*iter) == mate)
                {
                    is_mate = true;
                    continue;
                }
                if(is_mate)
                {
                    value+=*iter;
                }
                else
                {
                    key+=*iter;
                }
            }
            //放入config map
            if(key != "")
            {
                std::pair<std::string,std::string> kv(key,value);
                config.insert(kv);
            }
        }
        result = true;
    }


private:
    bool result;
    std::map<std::string,std::string> config;
};



#endif