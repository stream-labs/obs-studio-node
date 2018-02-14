#include <node.h>
#include <string>

extern std::string g_moduleDirectory;

void replaceAll(std::string& str, const std::string& from, const std::string& to);
void nodeobs_init(v8::Local<v8::Object> exports);
