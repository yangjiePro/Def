#--
 Def 语言测试执行文件
--#


# 模块导入测试

# # 导入系统模块
# import "fs" 
# print(fs)
# # 调用系统函数
# txt : fs.read("item/math.d")
# print(txt)

# 块结构
# node : {
# 	1+2
# 	n+m
# }

# print(node[1])

v : ("yangjie" 25)

printl(v)
printlr(v)
print(v)
printr(v)

# num : 1
# bl : num~=1

# print(bl)


# import "container.d" # 容器
# print container.li

# import "modules/math.d" # 运算
# print math.PI

# import "item/func.d" # 函数

# import "item/proc.d" # 处理器

# arr : (100 3.69 "yangjie")
# dt : [
# 	'name' "yangjie"
# 	'age' 25
# ]
# # arr.print()
# for dt k v
# 	print(k)
# 	print(v)
# ;


# 测试处理器
# proc.test{ arr }



# # 遍历容器
# proc.foreach{ arr k v
# 	print( k )
# 	print( v )
# }

# # 函数递归测试
# defun recur(i)
# 	if i=0
# 		return "bottom"
# 	;
# 	# print(i)
# 	recur(i-1)
# ;

# print( recur(200) )

