#ifndef DEF_VM_MODULE_H
#define DEF_VM_MODULE_H

/**
 * Def 模块加载
 */

#include <string>
#include <vector>
#include <map>

using namespace std;

#include "../object/object.h"

using namespace def::object;


namespace def {
namespace vm {


// 模块加载处理
class Module {

	public:

	map<string, ObjectModule*> _exist; // 已经加载的模块

	public:

	Module();

	// 判断并加载模块（模块已缓存或为系统模块则加载成功）
	ObjectModule* Load(string name, string file=""); 
	ObjectModule* LoadSys(string name); // 加载系统模块

	static string MatchFile(string name, string basefile=""); // 匹配模块文件

	ObjectModule* GetCache(string file); // 获取缓存模块
	void SetCache(string file, ObjectModule* cache); // 缓存模块
	void ClearCache(string name, string basefile=""); // 清除模块


};


} // end namespace vm
} // end namespace def



#endif
// --end-- DEF_MODULE_H