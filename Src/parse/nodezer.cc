
#include <fstream>
#include <string>
#include <vector>

#include "nodezer.h"
//#include "../Util/log.h"

using namespace std;

using namespace def::token;
using namespace def::node;


/**
 * 构造
 */
Nodezer::Nodezer(vector<Word>& w, NodeTree& n):
    words(w), node(n)
{
    Clear();
}


/**
 * 扫描单词 构建语法树
 */
void Nodezer::Scan()
{
    while(1){
        Read();
        if(cur.type==nullword.type){
            break;
        }
        cout << cur.value << endl;
    }

}




/****************** 语法分析器测试 *******************/


int main()
{
    cout << "\n";


    // 词法分析结果
    vector<Word> words;

    // 初始化词法分析器
    Tokenizer T(true, "./test.d", words);

    // 执行词法分析
    T.Scan();

    // 语法树
    NodeTree node;

    // 初始化语法分析器
    Nodezer N(words, node);

    // 执行语分析
    N.Scan();




    /*/ 打印词法分析结果
    for(int i=0; i<words.size(); i++){
    	Word wd = words[i];
		cout << wd.line << ","<< wd.posi << "  " << (int)wd.type << "  " << wd.value << endl;
    }
    */



    cout << "\n\n";


    /*

    Tokenizer TK("./Parser/test.d", true);
    vector <Word> words = TK.Scan();

    for(int i=0; i<words.size(); i++){
    	Word wd = words[i];
		cout << wd.line << ","<< wd.pos << "  " << (int)wd.type << "  " << wd.value << endl;
    }

    */

}




/**************** 语法分析器测试结束 *****************/