#include <iostream>
#include "gwn/config.h"
#include "gwn/log.h"
#include <yaml-cpp/yaml.h>

gwn::ConfigVar<int>::ptr g_int_value_config = 
    gwn::Config::LookUp("system.port",(int)8080,"system port");

gwn::ConfigVar<float>::ptr g_float_value_config = 
    gwn::Config::LookUp("system.value",(float)10.2f,"system value");

gwn::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
    gwn::Config::LookUp("system.int_vec", std::vector<int>{1,2}, "system int vec");

gwn::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    gwn::Config::LookUp("system.int_list", std::list<int>{1,2}, "system int list");

gwn::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    gwn::Config::LookUp("system.int_set", std::set<int>{1,2}, "system int set");

gwn::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    gwn::Config::LookUp("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

gwn::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    gwn::Config::LookUp("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

gwn::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    gwn::Config::LookUp("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");

void print_yaml(const YAML::Node& node,int level){
    if(node.IsScalar()){
        gwn_LOG_INFO(gwn_LOG_ROOT())<<std::string(level * 4, ' ')<<node.Scalar()<<"-"<<node.Tag()<<"-"<<level;
    }else if(node.IsNull()){
        gwn_LOG_INFO(gwn_LOG_ROOT())<<std::string(level * 4, ' ')<<"NULL -"<<node.Tag()<<"-"<<level;
    }else if(node.IsMap()){
        for(auto it = node.begin();it!=node.end();++it){
            gwn_LOG_INFO(gwn_LOG_ROOT())<<std::string(level * 4, ' ')<<it->first<<"-"<<it->second.Tag()<<"-"<<level;
            print_yaml(it->second,level+1);
        }
    }else if(node.IsSequence()){
        for(size_t i = 0; i < node.size(); ++i) {
            gwn_LOG_INFO(gwn_LOG_ROOT())<<std::string(level * 4, ' ')<<i<<"-"<<node[i].Tag()<<"-"<<level;
            print_yaml(node[i],level+1);
        }
    }
}

void test_yaml(){
    YAML::Node root = YAML::LoadFile("/root/gwn/bin/conf/log.yml");
    //YLAR_LOG_INFO(gwn_LOG_ROOT()) << root;

    print_yaml(root,0);

}

void test_config() {
    gwn_LOG_INFO(gwn_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    gwn_LOG_INFO(gwn_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
            gwn_LOG_INFO(gwn_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        gwn_LOG_INFO(gwn_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
            gwn_LOG_INFO(gwn_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        gwn_LOG_INFO(gwn_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }


    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/gaoweining/gwn/bin/conf/test.yml");
    gwn::Config::LoadFromYaml(root);

    gwn_LOG_INFO(gwn_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    gwn_LOG_INFO(gwn_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

void test_log() {
    static gwn::Logger::ptr system_log = gwn_LOG_NAME("system");
    gwn_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << gwn::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/gaoweining/gwn/bin/conf/log.yml");
    gwn::Config::LoadFromYaml(root);
    std::cout << "=============" << std::endl;
    std::cout << gwn::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << root << std::endl;
    gwn_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    gwn_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc, char **argv)
{

    //test_yaml();
    //test_config();
    test_log();
    return 0;
}