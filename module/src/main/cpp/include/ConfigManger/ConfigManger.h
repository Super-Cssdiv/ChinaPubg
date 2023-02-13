#ifndef Ini_RW
#define Ini_RW

struct item_t {
    char *name;
    char *value;
    item_t *next;
};

struct section_t {
    char *name;
    item_t *items;
    item_t *tail;
    section_t *next;
};

class ConfigManger {
public:
    char **section_names;
    char **item_names;
    section_t *sections;
    section_t *tail;
    char file[512];

    char *readString(const char *section, const char *key, char *deValue);

    int readInteger(const char *section, const char *key, int deValue);

    float readFloat(const char *section, const char *key, float deValue);

    bool readBoolean(const char *section, const char *key, bool deValue);

    void putString(const char *section, const char *key, char *value);

    void putInteger(const char *section, const char *key, int value);

    void putFloat(const char *section, const char *key, float value);

    void putBoolean(const char *section, const char *key, bool value);

    void removeKey(const char *section, const char *key);

    void clear( );

    void free( );
};
//保存文件
void save_file(ConfigManger *ini);
//释放Section
void free_section_names(ConfigManger *ini);
//释放Item
void free_item_names(ConfigManger *ini);
//查找Section
section_t *find_section(ConfigManger *ini, char *name);
//查找Item
item_t *find_item(section_t *s, char *name);
//添加Section
section_t *add_section(ConfigManger *ini, char *name);
//添加Item
int add_item(ConfigManger *ini, char *section, char *item, char *value);
//删除Section
int rm_section(ConfigManger *ini, section_t *s);
//初始化获取Ini
ConfigManger *ini_load(const char *path);
//遍历所有Section
char **ini_list_sections(ConfigManger *ini);
//遍历所有Item
char **ini_list_items(ConfigManger *ini, char *section);
//读取指定Item
char *ini_read(ConfigManger *ini, char *section, char *item);
//写入指定Item
int ini_write(ConfigManger *ini, char *section, char *item, char *value);
//删除指定Item
int ini_remove(ConfigManger *ini, char *section, char *item);

#endif
