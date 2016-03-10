/**
 *
 */

#include "gen.h"

#include "../core/ast.h"
#include "../core/error.h"
#include "../parse/analysis.h"

using namespace std;
using namespace llvm;
using namespace def::core;
using namespace def::compile;
using namespace def::parse;



/**
 * 获取变量
 */
Value* Gen::getValue(const string & name)
{
    auto rel = values.find(name);
    if (rel != values.end()) {
        return rel->second;
        // FATAL("codegen: Cannot find variable '"+name+"' !")
    }
    return nullptr;
}


/**
 * 放置变量
 * 返回旧变量
 */
Value* Gen::putValue(const string & name, Value* v)
{
    Value* old = getValue(name);
    values[name] = v;
    return old;
}


/**
 * 将 AST 处理成可以作为函数参数使用
 */
Value* Gen::varyPointer(AST* ast)
{
    // AST* ast = (AST*)p;
    Value *v = ast->codegen(*this);

    Value *val = varyPointer(v);

    // 如果是变量，则重赋值
    if (auto *vari = dynamic_cast<ASTVariable*>(ast)) {
        putValue(vari->name, val);
    }

    return val;
}
Value* Gen::varyPointer(Value* val)
{
    // Value *val = (Value *)v;
    llvm::Type* ty = val->getType();
    // 结构类型的值，重新分配内存

    if (! isa<PointerType>(ty) ){
        Value *aoc = builder.CreateAlloca(ty);
        builder.CreateStore(val, aoc); // 拷贝
        val = aoc;
    }
    

    return val;

}




/**
 * 数据载入
 */
Value* Gen::createLoad(AST* p)
{
    AST* ast = (AST*)p;
    // def::parse::Type *ty = Analysis::getType(ast);
    Value *val = ast->codegen(*this);

    return createLoad(val);

}
Value* Gen::createLoad(Value* val)
{
    // Value *val = (Value*)v;
    llvm::Type *ty = val->getType();

    // Store 操作
    if (isa<StoreInst>(val)) {
        auto * stis = (StoreInst*)val;
        return stis->getValueOperand();
    }

    // 指针节点
    if( isa<PointerType>(ty) ) {
        // 需要从地址载入数据
        return builder.CreateLoad(val);
    }
    
    return val;

}


/**
 * 创建函数
 */
Function* Gen::createFunction(AST* p)
{
    ASTFunctionCall* call = (ASTFunctionCall*)p;
    string fname = call->fndef->ftype->name;
    string idname = call->fndef->getIdentify();
    // 查找缓存
    Function *func = module.getFunction(idname);
    if(func){
        // cout << "module.getFunction " << fname << endl;
        return func; // 返回缓存
    }
    
    // 保存旧的变量
    auto old_values = values;
    values.clear();

    // 变量栈
    vector<string> cpt_name;
    vector<def::core::Type*> cpt_type;
    
    // 类实例变量（非静态）
    if (call->fndef->belong && ! call->fndef->is_static_member) {
        cpt_name.push_back(DEF_MEMFUNC_ISTC_PARAM_NAME);
        cpt_type.push_back(call->fndef->belong->type);
    }

    // 捕获的变量
    for (auto &p : call->fndef->cptvar) { // 设置参数名称
        cpt_name.push_back(p.first);
        cpt_type.push_back(p.second);
    }

    // 创建函数类型
    FunctionType *fty = (FunctionType*)fixType( 
        call->fndef->ftype, 
        & cpt_type
    );
    
    // 创建函数
    func = Function::Create(fty, Function::ExternalLinkage, idname, &module);

    // 设置函数参数
    int idx = 0;
    int idx2 = 0;
    int cptnum = cpt_name.size();
    // cout << "values  = = = = "+call->fndef->ftype->name+" = = = = : " << endl;
    for (auto &Arg : func->args()) { // 设置参数
        string name;
        if (idx<cptnum) {
            // 捕获的外层变量
            name = cpt_name[idx];
        } else {
            // 实际参数
            name = call->fndef->ftype->tabs[idx2];
            idx2++;
        }
        Arg.setName(name);
        values[name] = &Arg;
        idx++;
        // cout << "values[name]: " << name << endl;
    }
    
    // 创建函数体
    if (!call->fndef->body) {
        FATAL("codegen: Cannot find function '"+fname+"' body !")
    }

    // 缓存旧的插入点
    BasicBlock *old_block = builder.GetInsertBlock();
    BasicBlock *new_block = BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(new_block);
    Value* last(nullptr);
    AST* tail(nullptr);
    for (auto &li : call->fndef->body->childs) {
        if (Value* v = li->codegen(*this)) {
            last = v;
            tail = li;
            //if (isa<ReturnInst>(last)) {
                // break;
            //}
        }
    }

    // 是否为构造函数
    if (! last || call->fndef->is_construct) {
        builder.CreateRetVoid();
    }else if ( ! isa<ReturnInst>(last)) {
        // 函数体最后一句自动成为返回值，返回值必须 Load
        // cout << "! isa<ReturnInst>(last)" << endl;
        // builder.CreateRet(last);
        builder.CreateRet(
           createLoad(last)
        );
    }else {
    }

    // 插入点 复位
    builder.SetInsertPoint(old_block);
    
    // 复位变量栈
    values = old_values;

    return func;
}



/**
 * 类型转换
 */
llvm::Type* Gen::fixType(def::core::Type* ty, vector<def::core::Type*>* append)
{

#define ISTY(T) def::core::Type##T* obj = dynamic_cast<def::core::Type##T*>(ty)

    // 原子类型
    if (ISTY(Nil)) {
        return builder.getVoidTy();
    } else if (ISTY(Bool)) {
        return builder.getInt1Ty();
    } else if (ISTY(Int)) {
        return builder.getInt32Ty();
    } else if (ISTY(Float)) {
        return builder.getFloatTy();
    } else if (ISTY(Char)) {
        return builder.getInt32Ty();
    } else if (ISTY(String)) {
        // return PointerType::get(builder.getInt8Ty(), 0);
        return builder.getInt8PtrTy();
    }
    
    // 引用类型
    if (ISTY(Refer)) {
        return builder.getInt32Ty();
    }

    // 函数类型
    if (ISTY(Function)) {
        std::vector<llvm::Type*> ptys;
        // 如果有追加的
        if (append) {
            for (auto &p : *append) {
                auto *pty = fixType(p);
                // 追加的类型始终以指针方式传递
                if ( ! isa<PointerType>(pty)) {
                    pty = PointerType::get(pty, 0);
                }
                ptys.push_back( pty ); // 参数类型
            }
        }
        for (auto &p : obj->types) {
            auto *pty = fixType(p);
            // 类以指针传递，基本类型值传递
            if (dynamic_cast<TypeStruct*>(p)) {
                if (!isa<PointerType>(pty)) {
                    pty = PointerType::get(pty, 0);
                }
            }
            ptys.push_back( pty ); // 参数类型
        }
        // 返回结构类型值，也返回地址
        auto rty = fixType(obj->ret);
        // rty = rty ? rty : builder.getVoidTy();
        /*
        if (dynamic_cast<TypeClass*>(obj->ret)) {
            rty = PointerType::get(rty, 0);
        }*/
        return FunctionType::get(rty, ptys, false);
    }

    // Struct 类类型
    if (ISTY(Struct)) {

        // 用开头的点号标识函数
        string name = obj->getIdentify();

        // 搜索是否已经创建
        auto* scty = module.getTypeByName(name);
        if(scty){
            return scty;
        }

        // 创建类类型
        scty = StructType::create(context, name);
        // 子类型
        std::vector<llvm::Type*> ptys;
        for (auto &p : obj->types) {
            ptys.push_back( fixType(p) ); // 参数类型
        }
        scty->setBody(ptys);
        // auto* llty = StructType::get(context, ptys, false);
        // scty->setName(obj->name);

        return scty;

        //return llty;
    }
    


#undef ISTY

}


// 内建函数类型转换
llvm::Type* Gen::fixBuiltinFunctionType(def::core::TypeFunction* fty)
{
    std::vector<llvm::Type*> ptys;
    for (auto &p : fty->types) {
        auto *pty = fixType(p);
        // if (1 || dynamic_cast<TypeStruct*>(p)) {
        // if ( ! isa<PointerType>(pty)) {
            // 通过指针传递结构类型参数
            // pty = PointerType::get(pty, 0);
        // }
        ptys.push_back( pty ); // 参数类型
    }
    // 返回结构类型值，也返回地址
    auto rty = fixType(fty->ret);
    // rty = rty ? rty : builder.getVoidTy();
    /*
    if (dynamic_cast<TypeClass*>(obj->ret)) {
        rty = PointerType::get(rty, 0);
    }*/
    return FunctionType::get(rty, ptys, false);
}