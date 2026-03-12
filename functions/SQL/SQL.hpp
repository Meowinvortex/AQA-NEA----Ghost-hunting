#ifndef SQL_H
#define SQL_H
#include<pqxx/pqxx>
#include "../struct.hpp"

namespace GH{
namespace SQL{
    std::string load_foreign_map(int map_id, std::string share_code);
    void change_share_code(int map_id, std::string share_code);
    sf::Texture load_texture(int theme, int index);
    pqxx::result load_all_theme(std::string theme);
    void save_asset(std::string asset_type, GH::CREATE::obj& asset, int map_id, int asset_index);
    void delete_map_data(int map_id);
    void save_map_size(int map_id, int objects, int walls, int floors, int rooms, int light_systems, int hiding, int ambient);
    bool map_empty(int map_id);
    pqxx::result load_asset(int map_id, std::string asset_type);
    std::vector<int> get_map_size(int map_id);
    pqxx::result get_map_names(std::string author);
    void save_light_systems(std::map<int,GH::CREATE::light_system>& light_systems, int map_id);
    std::pair<pqxx::result,pqxx::result> load_internal_system(int map_id, int light_system_id);
    std::string reg(char u[], char p[]);
    std::string log_in(char u[], char p[]);
    pqxx::result get_user_info(char u[]);
    void make_map(std::string user, int map_amount);
    void change_map_name(int map_id, std::string name);
}
}


#endif